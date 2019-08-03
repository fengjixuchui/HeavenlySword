CODE_DIR		= ..

job:
	$(MAKE) -C $(CODE_DIR)/job/src/ppu/jobapi VERSION=release
	$(MAKE) -C $(CODE_DIR)/job/src/spu/jobapi -f plugincrt0.makefile VERSION=release
	$(MAKE) -C $(CODE_DIR)/job/src/spu/jobapi -f jobcrt0.makefile VERSION=release
	$(MAKE) -C $(CODE_DIR)/job/src/spu/jobapi VERSION=release
	$(MAKE) -C $(CODE_DIR)/job/src/ppu/jobapi
	$(MAKE) -C $(CODE_DIR)/job/src/spu/jobapi -f plugincrt0.makefile
	$(MAKE) -C $(CODE_DIR)/job/src/spu/jobapi -f jobcrt0.makefile
	$(MAKE) -C $(CODE_DIR)/job/src/spu/jobapi

sample:
	$(MAKE) -C $(CODE_DIR)/job/samples VERSION=release

all: job sample
	

clean: job-clean

cleanall: job-clean sample-clean
	
job-clean:
	$(MAKE) -C $(CODE_DIR)/job/src/ppu/jobapi clean VERSION=release
	$(MAKE) -C $(CODE_DIR)/job/src/spu/jobapi -f plugincrt0.makefile clean VERSION=release
	$(MAKE) -C $(CODE_DIR)/job/src/spu/jobapi -f jobcrt0.makefile clean VERSION=release
	$(MAKE) -C $(CODE_DIR)/job/src/spu/jobapi clean VERSION=release
	$(MAKE) -C $(CODE_DIR)/job/src/ppu/jobapi clean
	$(MAKE) -C $(CODE_DIR)/job/src/spu/jobapi -f plugincrt0.makefile clean
	$(MAKE) -C $(CODE_DIR)/job/src/spu/jobapi -f jobcrt0.makefile clean
	$(MAKE) -C $(CODE_DIR)/job/src/spu/jobapi clean

sample-clean:
	$(MAKE) -C $(CODE_DIR)/job/samples clean
