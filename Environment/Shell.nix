{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {

	# Disable all hardening flags (otherwise gcc won't compile)
	hardeningDisable = [ "all" ];

	# Dependencies
	buildInputs = [
		pkgs.file
		# Run
		pkgs.qemu
		pkgs.ovmf
		# Debug
		pkgs.gdb
		pkgs.bochs
		# Building
		pkgs.gnumake
		pkgs.nasm
		pkgs.gcc
		pkgs.clang-tools
		pkgs.lld
		pkgs.git
		pkgs.gptfdisk
		pkgs.mtools
		pkgs.dosfstools
		pkgs.guestfs-tools
		# Toolchain
		pkgs.wget
		pkgs.texinfo
		pkgs.bison
		pkgs.flex
		pkgs.gmp
		pkgs.mpfr
		pkgs.libmpc
	];

	# Shell parameters
	shellHook = ''
		export PS1="\[\033[1;32m\][nix-shell:\w]\$\[\033[0m\] "
	'';
}
