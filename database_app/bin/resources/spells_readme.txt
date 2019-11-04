Structure for spells database
tabel maker command
spells 1 0 0 1 0 1 1 0 1 4 4 0 4 1 1 4 1 1 4 1 1 4 1 1 4 4 0 4 4 0 4 1 0 4 1 1 4 1 1 4 1 1 4 1 1 4 4 0 4 4 0 4 1 0 4 4 1 4 4 1 4 4 1 4 4 1 4 2 1 1 0 1 4 4 1 16 6 1

//first is the spell data
1 byte char spell visual, not sorted
1 byte char spell type, sorted
1 byte char spell cc type, sorted
4 bytes float cc duration, not sorted

//The first effect
4 bytes int the self activator, sorted
4 bytes int the remote activator, sorted
4 bytes int spell action, sorted
4 bytes int scalar, sorted
4 bytes float spell value, not sorted
4 bytes float duration, not sorted
4 bytes int target type, not sorted

//The second effect
4 bytes int the self activator, sorted
4 bytes int the remote activator, sorted
4 bytes int spell action, sorted
4 bytes int scalar, sorted
4 bytes float spell value, not sorted
4 bytes float duration, not sorted
4 bytes int target type, not sorted

4 bytes float spell mana cost, sorted
4 bytes float spell cool down, sorted
4 bytes float spell cast time, sorted
4 bytes float spell range, sorted

//spell done now for different stuff
4 bytes unsigned owner id, sorted
1 bytes char the state of the spell(equiped, inventory, for sale, ect), sorted
4 bytes float a state specific value(which slot, sale price, ect), sorted
16 bytes string name of the item, sorted



