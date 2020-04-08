/*
 * Copyright (c) 2020, Liav A. <liavalb@hotmail.co.il>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <Kernel/ACPI/ACPIParser.h>
#include <Kernel/CommandLine.h>
#include <Kernel/Net/E1000NetworkAdapter.h>
#include <Kernel/Net/RTL8139NetworkAdapter.h>
#include <Kernel/PCI/IOAccess.h>
#include <Kernel/PCI/Initializer.h>
#include <Kernel/PCI/MMIOAccess.h>
#include <LibBareMetal/IO.h>

namespace Kernel {
namespace PCI {

static Initializer* s_pci_initializer;

Initializer& Initializer::the()
{
    if (s_pci_initializer == nullptr) {
        s_pci_initializer = new Initializer();
    }
    return *s_pci_initializer;
}
void Initializer::initialize_pci_mmio_access(PhysicalAddress mcfg)
{
    MMIOAccess::initialize(mcfg);
    detect_devices();
}
void Initializer::initialize_pci_io_access()
{
    IOAccess::initialize();
    detect_devices();
}

void Initializer::detect_devices()
{
    enumerate_all([&](const Address& address, ID id) {
        klog() << "PCI: device @ " << String::format("%w", address.seg()) << ":" << String::format("%b", address.bus()) << ":" << String::format("%b", address.slot()) << "." << String::format("%d", address.function()) << " [" << String::format("%w", id.vendor_id) << ":" << String::format("%w", id.device_id) << "]";
        E1000NetworkAdapter::detect(address);
        RTL8139NetworkAdapter::detect(address);
    });
}

void Initializer::test_and_initialize(bool disable_pci_mmio)
{
    if (disable_pci_mmio) {
        if (test_pci_io()) {
            initialize_pci_io_access();
        } else {
            klog() << "No PCI Bus Access Method Detected, Halt!";
            ASSERT_NOT_REACHED(); // NO PCI Access ?!
        }
        return;
    }
    if (test_acpi()) {
        if (test_pci_mmio()) {
            initialize_pci_mmio_access_after_test();
        } else {
            if (test_pci_io()) {
                initialize_pci_io_access();
            } else {
                klog() << "No PCI Bus Access Method Detected, Halt!";
                ASSERT_NOT_REACHED(); // NO PCI Access ?!
            }
        }
    } else {
        if (test_pci_io()) {
            initialize_pci_io_access();
        } else {
            klog() << "No PCI Bus Access Method Detected, Halt!";
            ASSERT_NOT_REACHED(); // NO PCI Access ?!
        }
    }
}
Initializer::Initializer()
{
}
bool Initializer::test_acpi()
{
    if ((kernel_command_line().contains("noacpi")) || !ACPI::Parser::the().is_operable())
        return false;
    else
        return true;
}

bool Initializer::test_pci_io()
{
    klog() << "Testing PCI via manual probing... ";
    u32 tmp = 0x80000000;
    IO::out32(PCI_ADDRESS_PORT, tmp);
    tmp = IO::in32(PCI_ADDRESS_PORT);
    if (tmp == 0x80000000) {
        klog() << "PCI IO Supported!";
        return true;
    }

    klog() << "PCI IO Not Supported!";
    return false;
}

bool Initializer::test_pci_mmio()
{
    return !ACPI::Parser::the().find_table("MCFG").is_null();
}

void Initializer::initialize_pci_mmio_access_after_test()
{
    initialize_pci_mmio_access(ACPI::Parser::the().find_table("MCFG"));
}

void Initializer::dismiss()
{
    if (s_pci_initializer == nullptr)
        return;
    klog() << "PCI Subsystem Initializer dismissed.";
    delete s_pci_initializer;
    s_pci_initializer = nullptr;
}

Initializer::~Initializer()
{
}

}
}
