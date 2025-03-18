#include <linux/fs.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "sharedmem"
#define SHARED_MEM_SIZE PAGE_SIZE

static void *shared_mem;

/* mmap 操作，将内核中分配的内存映射到用户空间 */
static int sharedmem_mmap(struct file *filp, struct vm_area_struct *vma) {
  unsigned long pfn = virt_to_phys(shared_mem) >> PAGE_SHIFT;
  int ret;

  /* 限制映射大小不能超过分配的共享内存大小 */
  if ((vma->vm_end - vma->vm_start) > SHARED_MEM_SIZE)
    return -EINVAL;

  /* 删除对 vm_flags 的修改，因为在新版内核中它是只读的 */
  // vma->vm_flags |= VM_IO;

  vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

  ret = remap_pfn_range(vma, vma->vm_start, pfn, vma->vm_end - vma->vm_start,
                        vma->vm_page_prot);
  if (ret)
    return ret;

  return 0;
}

/* read 操作，从共享内存中读取数据 */
static ssize_t sharedmem_read(struct file *filp, char __user *buf, size_t count,
                              loff_t *ppos) {
  size_t available;

  /* 计算从当前偏移到共享内存末尾还有多少数据 */
  if (*ppos >= SHARED_MEM_SIZE)
    return 0;

  available = SHARED_MEM_SIZE - *ppos;
  if (count > available)
    count = available;

  /* 将共享内存中的数据拷贝到用户空间 */
  if (copy_to_user(buf, (char *)shared_mem + *ppos, count))
    return -EFAULT;

  *ppos += count;
  return count;
}

static const struct file_operations sharedmem_fops = {
    .owner = THIS_MODULE,
    .mmap = sharedmem_mmap,
    .read = sharedmem_read,
};

static struct miscdevice sharedmem_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEVICE_NAME,
    .fops = &sharedmem_fops,
};

static int __init sharedmem_init(void) {
  int ret;

  /* 分配一页共享内存 */
  shared_mem = kmalloc(SHARED_MEM_SIZE, GFP_KERNEL);
  if (!shared_mem)
    return -ENOMEM;
  memset(shared_mem, 0, SHARED_MEM_SIZE);

  /* 可以在模块加载时预写入一段测试数据，方便测试 read 接口 */
  strncpy(shared_mem, "Hello, shared memory from kernel module!",
          SHARED_MEM_SIZE - 1);

  ret = misc_register(&sharedmem_dev);
  if (ret) {
    kfree(shared_mem);
    return ret;
  }

  pr_info("sharedmem module loaded\n");
  return 0;
}

static void __exit sharedmem_exit(void) {
  misc_deregister(&sharedmem_dev);
  kfree(shared_mem);
  pr_info("sharedmem module unloaded\n");
}

module_init(sharedmem_init);
module_exit(sharedmem_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huigang Wang");
MODULE_DESCRIPTION("Linux内核模块共享内存示例(含 read 接口)");
