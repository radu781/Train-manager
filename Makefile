all:
	$(MAKE) -C Server
	$(MAKE) -C Client

clean:
	$(MAKE) clean -C Server
	$(MAKE) clean -C Client
