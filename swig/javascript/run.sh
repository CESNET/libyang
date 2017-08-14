dir=$(pwd)
echo $dir
line_number=$(grep -Fn 'LYEXT_PLUGINS_DIR' src/extensions_config.h | cut -f1 -d:)

# set new libyang plugin path
sed -i "${line_number}s/.*/#define LYEXT_PLUGINS_DIR \"${dir//\//\\/}\/plugin\"/" src/extensions_config.h
