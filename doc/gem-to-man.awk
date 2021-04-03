#!/usr/bin/env -S awk -f

BEGIN {
    preformatted=0
}

# Empty lines
/^$/ {
    print "."
    next
}

# Level 2 headings become section headings.
/^##\s/ {
    print ".SH"
    sub(/^##\s/, "")
    print toupper($0)
    next
}

# Level 3 headings become subsection headings.
/^###\s/ {
    sub(/^###\s/, "")
    print ".SS \"" $0 "\""
    next
}

# Lists
/^\*\s/ {
    print ".IP \\(bu 3"
    sub(/^\*\s/, "")
    print
    next
}

# Preformatted text
/^```/ {
    # We simply place indent macros to make
    # it stand out a little.
    if (!preformatted)
    {
        print ".RS"
        preformatted=1
    }
    else
    {
        preformatted=0
        print ".RE"
    }
    next
}

# Links
/^=>/ {
    # Strips => prefix, and separates URL and link
    # title.
    sub(/^=>/, "")
    url=$1
    $1=""
    title=$0

    print ".IP"

    # Print either URL on its own or Title: URL if we
    # have a title.
    if (title == "")
    {
        print url
    }
    else
    {
        gsub(/^\s/, "", title)
        print title ": " url
    }

    next
}

# Paragraphs (default)
{
    print ".PP"
    print
}
