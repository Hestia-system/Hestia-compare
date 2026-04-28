from pathlib import Path

from SCons.Script import Import


Import("env")

pio_home = Path.home() / ".platformio"
candidate_bins = [
    pio_home / "tools" / "toolchain-riscv32-esp" / "bin",
    pio_home / "packages" / "toolchain-riscv32-esp" / "bin",
]
candidate_bins.extend(
    sorted((pio_home / "packages").glob("toolchain-riscv32-esp*/bin"))
)

for bin_dir in candidate_bins:
    compiler_prefix = bin_dir / "riscv32-esp-elf"
    if Path(f"{compiler_prefix}-g++").is_file():
        env.PrependENVPath("PATH", str(bin_dir))
        env.Replace(
            AR=f"{compiler_prefix}-gcc-ar",
            AS=f"{compiler_prefix}-as",
            CC=f"{compiler_prefix}-gcc",
            CXX=f"{compiler_prefix}-g++",
            RANLIB=f"{compiler_prefix}-gcc-ranlib",
            SIZETOOL=f"{compiler_prefix}-size",
        )
        break
else:
    raise RuntimeError("ESP32-C3 RISC-V toolchain not found by PlatformIO")
