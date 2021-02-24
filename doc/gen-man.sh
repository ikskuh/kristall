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

main() {
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

    # Write the head to the output file.
    cp "$man_head" "$man_output"

    # Insert last modified date (use last-modified date of help.gemini)
    last_modified=$(date -r "$gemtext_in" +"%Y-%M-%d")
    sed -i "$man_output" -e 's#\$(DATE)#'"$last_modified"'#g'

    # Convert gemtext about page to roff's man format:
    convert_gemtext "$gemtext_in" >> "$man_output"

    # Write the tail to the output file.
    cat "$man_tail" >> "$man_output"
}

# Simple converter for gemtext to roff
# Note that this is not a converter of "standard" gemtext, and is
# designed specifically for kristall's about page. Thus, this may not work
# as expected on other gemtext files (may require a little tweaking)
convert_gemtext() {
    gem_in="$(cat $1 | tail -n +7 | head -n -7)"

    # Replace [Text] with bold text.
    gem_in="$(echo "$gem_in" | sed -Ee 's#\[([^]]*)\]#\\fB\1\\fR#g')"

    # Replace *Text* or _text_ with italic (not bold to help differentiate with the above)
    # (Regex is derived from one in geminirenderer.cpp)
    gem_in="$(echo "$gem_in" | sed -Ee 's#(^|[.,!? ]+)[*_]([^*_ ]+[^*_]+[^*_ ]+)[*_]($|[.,!? ])#\1\\fI\2\\fR\3#g')"

    is_preformatted=false

    # Iterate over all the lines in the gemtext file
    while IFS= read -r line; do
        if echo "$line" | grep -q '^##\s'; then
            # Top level heading
            text="${line#'## '}"
            echo ".SH"
            echo "${text^^}"
        elif echo "$line" | grep -q '^###\s'; then
            # Level 2 heading
            text="${line#'### '}"
            echo ".SS"
            echo "$text"
        elif echo "$line" | grep -q '^\*\s'; then
            # Lists
            text="${line#'* '}"
            echo ".IP \(bu 3"
            echo "${text}"
        elif echo "$line" | grep -q '^\=>'; then
            # Links. We only show the URL itself if there is one.
            text="${line#'=>'}"
            title="$(echo "$text" | awk '{$1=""; print $0}')"
            text="$(echo "$text" | awk '{print $1}')"

            # Prefix with URL title if we have one
            if [[ -n "$title" ]]; then
                text="$title: $text"
            fi

            echo ".IP"

            # Echo the text, we also strip leading whitespace
            echo "$text" | sed -e 's#^[[:space:]]*##'
        elif echo "$line" | grep -q '^```'; then
            # Preformatted text. We just indent it
            if [[ "$is_preformatted" = false ]]; then
                echo ".RS"
                is_preformatted=true
            else
                echo ".RE"
                is_preformatted=false
            fi
        elif [[ -z "$line" ]]; then
            # Empty lines
            echo "."
        else
            # Regular paragraphs
            echo ".PP"
            echo "$line"
        fi
    done <<< "$gem_in"
}

main "$@"
