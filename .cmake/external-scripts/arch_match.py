import os
import sys
import struct

def get_binary_arch(filepath):
    if not os.path.isfile(filepath):
        return "Not a File (Directory)"

    real_filepath = os.path.realpath(filepath)

    with open(real_filepath, 'rb') as f:
        magic = f.read(4)
        if not magic:
            return "Unknown"

        # LINUX / EMBEDDED: ELF Binaries
        if magic == b'\x7fELF':
            # Offset 18 holds the 2-byte e_machine field
            f.seek(18)
            machine = struct.unpack('<H', f.read(2))[0]
            elf_machines = {
                3: 'x86_32',
                62: 'x86_64',
                40: 'arm32',
                183: 'arm64'
            }
            return elf_machines.get(machine, f"Unknown ELF ({machine})")

        # WINDOWS: PE Binaries
        elif magic[:2] == b'MZ':
            # Offset 60 contains the pointer to the PE Header offset
            f.seek(60)
            pe_offset = struct.unpack('<I', f.read(4))[0]
            # Jump to PE Header signature: "PE\0\0" (4 bytes) + 2 bytes to Machine
            f.seek(pe_offset + 4)
            machine = struct.unpack('<H', f.read(2))[0]
            pe_machines = {
                0x014c: 'x86_32',
                0x8664: 'x86_64',
                0x01c0: 'arm32',
                0xaa64: 'arm64'
            }
            return pe_machines.get(machine, f"Unknown PE ({hex(machine)})")

        # MACOS: Mach-O Binaries
        # Universal / Fat layout (multi-architecture container file)
        elif magic in (b'\xca\xfe\xba\xbe', b'\xbe\xba\xfe\xca'):
            # Swap endianness layout if needed
            fmt = '>' if magic == b'\xca\xfe\xba\xbe' else '<'
            num_archs = struct.unpack(f'{fmt}I', f.read(4))[0]
            archs_found = []
            
            # Common Mach-O architecture mappings
            macho_cpus = {
                (7, False): 'x86_32',
                (7 | 0x01000000, True): 'x86_64',
                (12, False): 'arm32',
                (12 | 0x01000000, True): 'arm64'
            }

            for _ in range(num_archs):
                cputype, cpusubtype = struct.unpack(f'{fmt}ii', f.read(8))
                # Skip offset/size metadata bytes
                f.read(12) 
                is_64 = (cputype & 0x01000000) != 0
                arch_name = macho_cpus.get((cputype, is_64), "Unknown Mach-O")
                archs_found.append(arch_name)
            return archs_found

        # Single Architecture Mach-O files (Thin binaries)
        elif magic in (b'\xfe\xed\xfa\xce', b'\xce\xfa\xed\xfe', b'\xfe\xed\xfa\xcf', b'\xcf\xfa\xed\xfe'):
            is_64 = magic in (b'\xfe\xed\xfa\xcf', b'\xcf\xfa\xed\xfe')
            fmt = '<' if magic in (b'\xce\xfa\xed\xfe', b'\xcf\xfa\xed\xfe') else '>'
            cputype = struct.unpack(f'{fmt}i', f.read(4))[0]

            macho_cpus = {
                7: 'x86_64' if is_64 else 'x86_32',
                12: 'arm64' if is_64 else 'arm32'
            }
            # Strip mask bit off 64-bit descriptors
            clean_cpu = cputype & ~0x01000000
            return macho_cpus.get(clean_cpu, f"Unknown Thin Mach-O ({cputype})")

    return "Unknown Format"

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: arch_match.py <path_to_binary> <expected_arch>")
        sys.exit(1)

    binary_path = sys.argv[1]
    expected_arch = sys.argv[2].lower()
    actual_res = get_binary_arch(binary_path)

    # Convert list outputs (from Mach-O Fat binaries) into validation comparisons
    actual_list = actual_res if isinstance(actual_res, list) else [actual_res]

    print(f"Checking:  {binary_path}")
    print(f"Expected:  {expected_arch}")
    print(f"Detected:  {', '.join(actual_list)}")

    if expected_arch in actual_list:
        print("PASS: Architecture validation successful!")
        sys.exit(0)
    else:
        print("FAIL: Discovered target architecture mismatch!")
        sys.exit(1)
