# Stoneforge Mods

Create one folder per mod:

- `mods/<modname>/mod.json`
- `mods/<modname>/sprites.json`
- `mods/<modname>/blocks.json`
- `mods/<modname>/items.json`
- `mods/<modname>/scripts/*.lua`
- `mods/<modname>/textures/*.png`

Example `mod.json`:

```json
{
  "id": "my_mod",
  "name": "My Mod",
  "version": "0.1.0",
  "scripts": ["scripts/main.lua"]
}
```

Lua callbacks supported in this first version:

- `onTick(payload)`
- `onBlockPlaced(payload)`
- `onBlockBroken(payload)`
- `onItemUsed(payload)`
- `onCraft(payload)`
