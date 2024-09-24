output=obj/luagen.h
cat > $output << EOF
#ifndef CODIM_LUAGEN_H
#define CODIM_LUAGEN_H

typedef struct LuagenInfo {
    unsigned long size;
    const char* name;
    const char* code;
} LuagenInfo;

const LuagenInfo luagen_entries[] = {
EOF


for lua_file in src/lua/*.lua; do
    # src/lua/what.lua => 'what'
    # src/lua/a/b/what.lua => 'a.b.what'
    name=`echo $lua_file | sed -e 's!^src/lua/!!' -e 's/\.lua$//g' -e 's!/!\.!g'`
    [ "$name" = codim ] || name="codim.$name"

    # bytecode=`luajit -b $lua_file - | xxd -i | tr -d '\n'`
    # bytecode_len=`echo $bytecode | wc -w`
    code=`cat $lua_file | sed 's/\\\\/\\\\\\\\/g'`
    code=${code//$'\n'/\\n}
    printf '    {%d, "%s", "%s"},\n' "$bytecode_len" "$name" "$code" >> $output

    # printf '    {%d, "%s", (char[]){ %s }},\n' "$bytecode_len" "$name" "$bytecode" >> $output
done

cat >> $output << EOF
    {0},
};

#endif // !CODIM_LUAGEN_H
EOF
