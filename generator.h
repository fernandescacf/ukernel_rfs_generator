#pragma once

#include <cstdint>
#include <string>
#include <fstream>
#include "parser.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

class Rfs
{
public:
    struct header
    {
        uint32_t type { 0 };
        uint32_t version { 0 };
        uint32_t arch { 0 };
        uint32_t machine { 0 };
        uint32_t fs_size { 0 };
        uint32_t script_off { 0 };
        uint32_t script_cmds { 0 };
        uint32_t ram_off { 0 };
        uint32_t irq_off { 0 };
        uint32_t devices_off { 0 };
        uint32_t devices_count { 0 };
        uint32_t names_off { 0 };
        uint32_t names_size { 0 };
        uint32_t files_off { 0 };
        uint32_t files_count { 0 };
    } m_header;

    struct cmd
    {
        uint32_t type { 0 };
        uint16_t prio { 0 };
        uint16_t priv { 0 };
        uint32_t file_off { 0 };
        uint32_t cmd_off { 0 };
    } m_cmd;

    struct ram
    {
        uint32_t addr { 0 };
        uint32_t size { 0 };
    } m_ram;

    struct irq
    {
        uint32_t priv   { 0 };
        uint32_t shared { 0 };
    } m_irq;

    struct device
    {
        uint32_t addr { 0 };
        uint32_t size { 0 };
        uint32_t access { 0 };
        uint32_t name_off { 0 };
    } m_device;

    struct file
    {
        uint32_t type { 0 };
        uint32_t size { 0 };
        uint32_t data_off { 0 };
        uint32_t name_off { 0 };
    } m_file;
};

bool GenerateRfs(const RfsTree& rfsTree, std::string&& outName)
{
    std::fstream outFile(outName, std::fstream::out | std::ofstream::trunc | std::fstream::binary);

    Rfs rfs;
    // Fill Header Information
    {
        rfs.m_header.type = rfsTree.getHeader().RfsType;
        rfs.m_header.version = rfsTree.getHeader().m_shVersion;
        rfs.m_header.arch = rfsTree.getHeader().m_shArch;
        rfs.m_header.machine = rfsTree.getHeader().m_shMach;

        // Fill Header entries for the start up script
        rfs.m_header.script_cmds = rfsTree.getCmds().size();
        rfs.m_header.script_off = sizeof(rfs.m_header);

        // Fill Header ram Information
        rfs.m_header.ram_off = rfs.m_header.script_off + sizeof(Rfs::cmd) * rfs.m_header.script_cmds;

        // Fill Header interrupt Information
        rfs.m_header.irq_off = rfs.m_header.ram_off + sizeof(Rfs::ram);

        // Fill Header devices Information
        rfs.m_header.devices_off = rfs.m_header.irq_off + sizeof(Rfs::irq);
        rfs.m_header.devices_count = rfsTree.getDevices().size();

        // Fill Header names/strings Information
        rfs.m_header.names_off = rfs.m_header.devices_off + sizeof(Rfs::device) * rfs.m_header.devices_count;
        rfs.m_header.names_size = rfsTree.getStrTable().getSize();

        // Fill Header files Information
        rfs.m_header.files_off = ALIGN_UP((rfs.m_header.names_off + rfs.m_header.names_size), 0x4);
        rfs.m_header.files_count = rfsTree.getFiles().size();
    }

    // Start generation the raw file system
    {
        // Write Header to RFS
		outFile.write((const char*)&rfs.m_header, sizeof(rfs.m_header));

        // Write Startup Script to RFS
        auto& cmds = rfsTree.getCmds();
        for (auto const& cmd : cmds)
        {
            rfs.m_cmd.type = cmd->getType();
            rfs.m_cmd.priv = cmd->getPrivilege();
            rfs.m_cmd.prio = cmd->getPriority();
            rfs.m_cmd.cmd_off = cmd->getArgsOffset(); //cmd->getNameOffset();
            rfs.m_cmd.file_off = rfs.m_header.files_off + sizeof(Rfs::file) * cmd->getFile()->getIndex();
            // Write Startup Commando to RFS
            outFile.write((const char*)&rfs.m_cmd, sizeof(rfs.m_cmd));
        }

        // Write Ram information to RFS
		rfs.m_ram.addr = rfsTree.getRam().m_address;
		rfs.m_ram.size = rfsTree.getRam().m_size;
		// Write to RFS
		outFile.write((const char*)&rfs.m_ram, sizeof(rfs.m_ram));

		// Write Interrupts information to RFS
		rfs.m_irq.priv = rfsTree.getInterrupts().m_priv;
		rfs.m_irq.shared = rfsTree.getInterrupts().m_shared;
		// Write to RFS
		outFile.write((const char*)&rfs.m_irq, sizeof(rfs.m_irq));

		// Write Device List to RFS
        auto& devices = rfsTree.getDevices();
		for (auto const& dev : devices)
		{
			rfs.m_device.addr = dev->getAddress();
			rfs.m_device.size = dev->getSize();
			rfs.m_device.name_off = dev->getNameOffset();
			rfs.m_device.access = 0;
			// Write device to RFS
			outFile.write((const char*)&rfs.m_device, sizeof(rfs.m_device));
		}

        // Write names/string pool to RFS
        auto& strings = rfsTree.getStrTable().getStrings();
        for (auto const& string : strings)
        {
            outFile.write((const char*)string->data(), (static_cast<std::streamsize>(string->size()) + 1));
        }

        // Aply padding if needed to align the next section to a 4 bytes address
        int pad = rfs.m_header.files_off - (rfs.m_header.names_off + rfs.m_header.names_size);
        if (pad > 0)
        {
            uint32_t dummy = 0;
            outFile.write((const char*)&dummy, pad);
        }
        
        // Write Files Headers to RFS
        auto& files = rfsTree.getFiles();
        uint32_t c_off = rfs.m_header.files_off + sizeof(Rfs::file) * rfs.m_header.files_count;
        for(auto const& file : files)
        {
            rfs.m_file.type = file->getType();
            rfs.m_file.size = file->getAlignSize();
            rfs.m_file.name_off = file->getNameOffset();
            rfs.m_file.data_off = c_off;
            outFile.write((const char*)&rfs.m_file, sizeof(rfs.m_file));
            c_off += rfs.m_file.size;
        }

        for(uint32_t i = 0; i < rfs.m_header.files_count; i++)
        {
            int fd = open(files.at(i)->getSource().c_str(), O_RDONLY);
            size_t len = ALIGN_UP(files.at(i)->getAlignSize(), 4096);
            char *elf = (char*)mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
            outFile.write((const char*)elf, files.at(i)->getAlignSize());

            munmap(elf, len);
            close(fd);
        }
    }
}