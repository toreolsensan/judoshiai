#!/bin/sh

if [ -x ./filter-po ]; then

make po COMMENT=YES

for file in *.po
do
    echo
    echo "*** $file ***"
    name=`basename "$file" .po`
    ./filter-po $file >$name.tmp
    mv $name.tmp $file
done

else

cat <<EOF >filter-po.c
#include <stdio.h>

int main(int argc, char *argv[])
{
        char line[256];
        int comment = 0;
        int ok = 1;

        if (argc < 2)
                return 1;

        FILE *f = fopen(argv[1], "r");
        if (!f) {
                perror(argv[1]);
                return 1;
        }

        while (fgets(line, sizeof(line), f)) {
                if (line[0] == '#')
                        comment = 1;
                else if (strncmp(line, "msgid", 5) == 0) {
                        ok = comment;
                        comment = 0;
                }
                if (ok || comment)
                        printf("%s", line);
                else
                        fprintf(stderr, "NOT USED: %s", line);
        }

        fclose(f);
}

EOF

gcc -o filter-po filter-po.c

fi
