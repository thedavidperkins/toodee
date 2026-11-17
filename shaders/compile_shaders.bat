@echo off

SET src="..\%1"

if not exist modules (
	mkdir modules
)

PUSHD modules

glslangValidator -V %src%

POPD