#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x10efe06b, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x11a13e31, __VMLINUX_SYMBOL_STR(_kstrtol) },
	{ 0x12da5bb2, __VMLINUX_SYMBOL_STR(__kmalloc) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0x5b121d68, __VMLINUX_SYMBOL_STR(class_unregister) },
	{ 0xb19fe77, __VMLINUX_SYMBOL_STR(device_destroy) },
	{ 0xd921e51c, __VMLINUX_SYMBOL_STR(proc_remove) },
	{ 0xaf963fcb, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0x46991692, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0xf16640bd, __VMLINUX_SYMBOL_STR(class_destroy) },
	{ 0xe686a91d, __VMLINUX_SYMBOL_STR(device_create) },
	{ 0x6bc3fbc0, __VMLINUX_SYMBOL_STR(__unregister_chrdev) },
	{ 0x4a5e996e, __VMLINUX_SYMBOL_STR(__class_create) },
	{ 0x90f8772e, __VMLINUX_SYMBOL_STR(__register_chrdev) },
	{ 0x91549e79, __VMLINUX_SYMBOL_STR(proc_create_data) },
	{ 0x91715312, __VMLINUX_SYMBOL_STR(sprintf) },
	{ 0xd0d8621b, __VMLINUX_SYMBOL_STR(strlen) },
	{ 0x50eedeb8, __VMLINUX_SYMBOL_STR(printk) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

