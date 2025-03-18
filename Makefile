# 定义关键路径变量
KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build
CURRENT_DIR := $(shell pwd)
BUILD_DIR := $(CURRENT_DIR)/build

# 内核模块配置
MODULE_NAME = shared_mem
obj-m := $(MODULE_NAME).o

# 用户空间程序配置
TEST_PROGRAM = test

all: modules user_program

modules: | $(BUILD_DIR)
	@echo "Building kernel module..."
	$(MAKE) -C $(KERNEL_DIR) M=$(CURRENT_DIR) modules
	@mv -f $(MODULE_NAME).ko $(BUILD_DIR)/
	@echo "Module built: $(BUILD_DIR)/$(MODULE_NAME).ko"

user_program: $(BUILD_DIR)/$(TEST_PROGRAM)

$(BUILD_DIR)/$(TEST_PROGRAM): test.c | $(BUILD_DIR)
	@echo "Building user program..."
	$(CC) $< -o $@
	@echo "User program built: $@"

$(BUILD_DIR):
	@mkdir -p $@

clean:
	@rm -rf $(BUILD_DIR)/*
	$(MAKE) -C $(KERNEL_DIR) M=$(CURRENT_DIR) clean
	@echo "Build files cleaned"

.PHONY: all modules user_program clean