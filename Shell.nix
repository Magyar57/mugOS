{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {

	# Disable all hardening flags (otherwise gcc won't compile)
	hardeningDisable = [ "all" ];
	
	# Dependencies
	buildInputs = [
		pkgs.file
		pkgs.gcc
		pkgs.gnumake
		pkgs.bison
		pkgs.flex
		pkgs.gmp
		pkgs.mpfr
		pkgs.libmpc
		pkgs.texinfo
		pkgs.wget
		pkgs.nasm
		pkgs.mtools
		pkgs.dosfstools
		pkgs.guestfs-tools
		pkgs.qemu
	];

	# Shell parameters
	shellHook = ''
		export PS1="\[\033[1;32m\][nix-shell:\w]\$\[\033[0m\] "
	'';
}
