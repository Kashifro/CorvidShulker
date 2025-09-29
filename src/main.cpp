#include <dlfcn.h>
#include <link.h>
#include <span>
#include <string>
#include <vector>
#include <cstdio>
#include <cstddef>
#include <format>
#include "main.h"
#include <libhat.hpp>

//fwd dec
class ItemStackBase;
class ShulkerBoxBlockItem;
class Level;


//ptr type
using ItemStackBase_toDebugString_t = std::string(*)(ItemStackBase* self);
using Shulker_appendHover_t = void(*)(ShulkerBoxBlockItem* self, const ItemStackBase& stack, Level& level, std::string& out, bool flag);

//org ptr
ItemStackBase_toDebugString_t ItemStackBase_toDebugString_orig = nullptr;
Shulker_appendHover_t ShulkerBoxBlockItem_appendFormattedHovertext_orig = nullptr;

//hook DebugString another mindless hook
std::string ItemStackBase_toDebugString_hook(ItemStackBase* self) {
     if (ItemStackBase_toDebugString_orig) {
        return ItemStackBase_toDebugString_orig(self);
    }
    return "DebugString unavailable";
}
//hook append
void ShulkerBoxBlockItem_appendFormattedHovertext_hook(ShulkerBoxBlockItem* self, const ItemStackBase& stack, Level& level, std::string& out, bool flag) {
    if (ShulkerBoxBlockItem_appendFormattedHovertext_orig)
        ShulkerBoxBlockItem_appendFormattedHovertext_orig(self, stack, level, out, flag);

    out.append("\n§2Preview Goes Here\n");
      if (ItemStackBase_toDebugString_orig) {
        std::string debug = ItemStackBase_toDebugString_orig(const_cast<ItemStackBase*>(&stack));
        out.append("\n§6Info:\n" + debug); // too much extra info and well it kinda is string hook but well debug lol xD
        //rest of the logic
    }
}
extern "C" [[gnu::visibility("default")]] void mod_preinit() {}
extern "C" [[gnu::visibility("default")]] void mod_init() {
    void* mcLib = dlopen("libminecraftpe.so", 0);
    if (!mcLib) { printf("[ShulkerPreview] failed to open libminecraftpe.so\n"); return; }

    std::span<std::byte> range1, range2;
    auto callback = [&](const dl_phdr_info& info) {
        if (auto h = dlopen(info.dlpi_name, RTLD_NOLOAD); dlclose(h), h != mcLib)
            return 0;
        range1 = {reinterpret_cast<std::byte*>(info.dlpi_addr + info.dlpi_phdr[1].p_vaddr), info.dlpi_phdr[1].p_memsz};
        range2 = {reinterpret_cast<std::byte*>(info.dlpi_addr + info.dlpi_phdr[2].p_vaddr), info.dlpi_phdr[2].p_memsz};
        return 1;
    };
    dl_iterate_phdr([](dl_phdr_info* info, size_t, void* data) {
        return (*static_cast<decltype(callback)*>(data))(*info);
    }, &callback);


    //ShulkerBoxBlockItem
    auto ZTS19ShulkerBoxBlockItem = hat::find_pattern(range1, hat::object_to_signature("19ShulkerBoxBlockItem")).get();
    auto _ZTI19ShulkerBoxBlockItem = hat::find_pattern(range2, hat::object_to_signature(ZTS19ShulkerBoxBlockItem)).get() - sizeof(void*);
    auto _ZTV19ShulkerBoxBlockItem = hat::find_pattern(range2, hat::object_to_signature(_ZTI19ShulkerBoxBlockItem)).get() + sizeof(void*);
    void** vt0 = reinterpret_cast<void**>(_ZTV19ShulkerBoxBlockItem);
    //append slot 53
    ShulkerBoxBlockItem_appendFormattedHovertext_orig =
        reinterpret_cast<Shulker_appendHover_t>(vt0[53]);
    vt0[53] = reinterpret_cast<void*>(&ShulkerBoxBlockItem_appendFormattedHovertext_hook);


    //ItemStackBase
    auto ZTS13ItemStackBase = hat::find_pattern(range1, hat::object_to_signature("13ItemStackBase")).get();
    auto _ZTI13ItemStackBase = hat::find_pattern(range2, hat::object_to_signature(ZTS13ItemStackBase)).get() - sizeof(void*);
    auto _ZTV13ItemStackBase = hat::find_pattern(range2, hat::object_to_signature(_ZTI13ItemStackBase)).get() + sizeof(void*);
    //slot7
     void** vt2 = reinterpret_cast<void**>(_ZTV13ItemStackBase);
    ItemStackBase_toDebugString_orig = 
       reinterpret_cast<ItemStackBase_toDebugString_t>(vt2[7]);
    vt2[7] = reinterpret_cast<void*>(&ItemStackBase_toDebugString_hook);

}