# Makefile for the drivers module

# Set the module name
MODULE := fs

FILES := vfs.c tfs.c fat32.c filesystems.c

SRC += $(patsubst %, $(MODULE)/%, $(FILES))
