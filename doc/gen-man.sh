#!/bin/bash
#
# gen-man
#
# Generates kristall's man page from the gemtext about:help file.
#
# How we do this:
# 0.) We insert a pre-defined "head" first, which contains flags, options, etc
#     which are not present in the Help file.
#
# 1.) Convert the main stuff the Help file to a roff format.
#
# 2.) Append a pre-defined "tail".

# Locations of the "head" and "tail"
man_head="./kristall-head.man"
man_tail="./kristall-tail.man"
man_output="./kristall.1"
gemtext_in="../src/about/help.gemini"
gemtext_converter="./gem-to-man.awk"

# Make sure we have all the stuff we need
if [[ ! -f "$man_head" ]]; then
    echo "man page head does not exist."
    exit -1
fi
if [[ ! -f "$man_tail" ]]; then
    echo "man page tail does not exist"
    exit -1
fi
if [[ -z "$man_output" ]]; then
    echo "No output file"
    exit -1
fi
if [[ ! -f "$gemtext_in" ]]; then
    echo "Input gemtext file does not exist"
    exit -1
fi
if [[ ! -f "$gemtext_converter" ]]; then
    echo "Gemtext converter script does not exist"
    exit -1
fi

# Write the head to the output file.
cp "$man_head" "$man_output"

# Insert last modified date (use last-modified date of help.gemini)
last_modified=$(date -r "$gemtext_in" +"%F")
sed -i "$man_output" -e 's#\$(DATE)#'"$last_modified"'#g'

# Some pre-processing before giving our gemtext to the awk script.
gem_in=$(
    # Read input file
    cat "$gemtext_in" |

    # Strip a few lines from beginning/end of file.
    tail -n +9 |
    head -n -9 |

    # First expression replaces all [Text like this] with bold text.
    # Second expression replaces text like *This* or _this_ with italic text.
    sed -E \
      -e 's#\[([^]]*)\]#\\fB\1\\fR#g' \
      -e 's#(^|[.,!? ]+)[*_]([^*_ ]+[^*_]+[^*_ ]+)[*_]($|[.,!? ])#\1\\fI\2\\fR\3#g'
)

# Convert gemtext to man format
echo "$gem_in" | "$gemtext_converter" >> "$man_output"

# Write the tail to the output file.
cat "$man_tail" >> "$man_output"
