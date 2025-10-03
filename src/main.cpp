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

// fwd dec
class ShulkerBoxBlockItem;
class Level;

// ptr type
using ItemStackBase_toDebugString_t = std::string (*)(void *self);
using Shulker_appendHover_t = void (*)(void *self, void *stack, void *level, std::string &out, bool flag);

// org ptr
ItemStackBase_toDebugString_t ItemStackBase_toDebugString_orig = nullptr;
Shulker_appendHover_t ShulkerBoxBlockItem_appendFormattedHovertext_orig = nullptr;

// hook DebugString another mindless hook
std::string ItemStackBase_toDebugString_hook(void *stack)
{
    std::string out;

    // keep the orig
    if (ItemStackBase_toDebugString_orig)
        out = ItemStackBase_toDebugString_orig(stack);

    std::byte *s = reinterpret_cast<std::byte *>(stack);

    // offset from slot7 pseudocode unf hardcoded
    void *mItem = *reinterpret_cast<void **>(s + 8);
    void *mUserData = *reinterpret_cast<void **>(s + 16);
    void *mBlock = *reinterpret_cast<void **>(s + 24);

    out.append("\n mItem: " + std::format("{:p}", mItem));
    out.append("\n mBlock: " + std::format("{:p}", mBlock));
    out.append("\n mUserData: " + std::format("{:p}", mUserData));
    return out;
}
// hook append
void ShulkerBoxBlockItem_appendFormattedHovertext_hook(
    void *self,
    void *stack,
    void *level,
    std::string &out,
    bool flag)
{
    if (ShulkerBoxBlockItem_appendFormattedHovertext_orig)
        ShulkerBoxBlockItem_appendFormattedHovertext_orig(self, stack, level, out, flag);

    std::byte *s = reinterpret_cast<std::byte *>(stack);

    void *mItem = *reinterpret_cast<void **>(s + 8);
    void *mUserData = *reinterpret_cast<void **>(s + 16); // is 0x0 when empty shulker and returns ptr if not
    void *mBlock = *reinterpret_cast<void **>(s + 24);
    // pure addresses hopefully

    if (!mUserData)
        return; // only display if has data

    out.append("\n mItem: " + std::format("{:p}", mItem));   // kinda sitting around here for now
    out.append("\n mBlock: " + std::format("{:p}", mBlock)); // - - -- - - - same actually this returns name as in minecraft:shulker_box but we only have ptr
    out.append("\n mUserData: " + std::format("{:p}", mUserData));
}

extern "C" [[gnu::visibility("default")]] void mod_preinit() {}
extern "C" [[gnu::visibility("default")]] void mod_init()
{
    void *mcLib = dlopen("libminecraftpe.so", 0);
    if (!mcLib)
    {
        printf("[ShulkerPreview] failed to open libminecraftpe.so\n");
        return;
    }

    std::span<std::byte> range1, range2;
    auto callback = [&](const dl_phdr_info &info)
    {
        if (auto h = dlopen(info.dlpi_name, RTLD_NOLOAD); dlclose(h), h != mcLib)
            return 0;
        range1 = {reinterpret_cast<std::byte *>(info.dlpi_addr + info.dlpi_phdr[1].p_vaddr), info.dlpi_phdr[1].p_memsz};
        range2 = {reinterpret_cast<std::byte *>(info.dlpi_addr + info.dlpi_phdr[2].p_vaddr), info.dlpi_phdr[2].p_memsz};
        return 1;
    };
    dl_iterate_phdr([](dl_phdr_info *info, size_t, void *data)
                    { return (*static_cast<decltype(callback) *>(data))(*info); }, &callback);

    // ShulkerBoxBlockItem
    auto ZTS19ShulkerBoxBlockItem = hat::find_pattern(range1, hat::object_to_signature("19ShulkerBoxBlockItem")).get();
    auto _ZTI19ShulkerBoxBlockItem = hat::find_pattern(range2, hat::object_to_signature(ZTS19ShulkerBoxBlockItem)).get() - sizeof(void *);
    auto _ZTV19ShulkerBoxBlockItem = hat::find_pattern(range2, hat::object_to_signature(_ZTI19ShulkerBoxBlockItem)).get() + sizeof(void *);
    void **vtshulk53 = reinterpret_cast<void **>(_ZTV19ShulkerBoxBlockItem);
    // append slot 53
    ShulkerBoxBlockItem_appendFormattedHovertext_orig =
        reinterpret_cast<Shulker_appendHover_t>(vtshulk53[53]);
    vtshulk53[53] = reinterpret_cast<void *>(&ShulkerBoxBlockItem_appendFormattedHovertext_hook);

    // ItemStackBase
    auto ZTS13ItemStackBase = hat::find_pattern(range1, hat::object_to_signature("13ItemStackBase")).get();
    auto _ZTI13ItemStackBase = hat::find_pattern(range2, hat::object_to_signature(ZTS13ItemStackBase)).get() - sizeof(void *);
    auto _ZTV13ItemStackBase = hat::find_pattern(range2, hat::object_to_signature(_ZTI13ItemStackBase)).get() + sizeof(void *);
    // slot7
    void **vtIstack07 = reinterpret_cast<void **>(_ZTV13ItemStackBase);
    ItemStackBase_toDebugString_orig =
        reinterpret_cast<ItemStackBase_toDebugString_t>(vtIstack07[7]);
    vtIstack07[7] = reinterpret_cast<void *>(&ItemStackBase_toDebugString_hook);

    // prolly have to drop it idk or not
}