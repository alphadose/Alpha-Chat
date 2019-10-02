.PHONY: default
default: help

all: client server
	@echo "* Done"

client:
	@echo "* Building Client"
	@gcc client.c -o client

server:
	@echo "* Building Server"
	@gcc server.c -o server

clean:
	@rm  client server 1>/dev/null 2>&1
	@echo "* Done"

help:
	@echo "Alpha-Chat: A chat ecosystem built purely in C"
	@echo ""
	@echo "* server: Build the server"
	@echo "* client: Build the client"
	@echo "* all:    Build the server and client"
	@echo "* clean:  Remove project binaries"
	@echo ""
