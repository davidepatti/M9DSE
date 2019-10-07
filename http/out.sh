#!/bin/sh

    pareto="./pareto.latest"
    rm -rf $pareto
    cat $(ls -t ../*pareto* | head -n1) | sed 's/\(\s.\)e/\1.00e/g; s/\(\s...\)e/\10e/g' > $pareto

    cat $pareto | 
    awk '
        /VGS/{print $2 "\t\t" $3 "\t\t" $4 "\t\t" $5 "\t\t" $6 "\t\t" $7 "\t\t" $8 "\t\t" $9 "\t\t" $10 "\t\t" $11 "\t\t" $12 "\t\t" $13 "\t\t" $14 "\t" $15 "\t" $16 "\t" $17 "\t" $18 "\t" $19 "\t" $20 "\t\t" $21}
        /^[0-9]/{print $0}
    ' > $pareto.txt

    cat $pareto | 
    awk '
        /VGS/{print substr($0, index($0,$2))}
        /^[0-9]/{print $0}
    ' | tr "." "," > $pareto.tsv

    cat $pareto | 
    awk '
        BEGIN {
            isFirstLine = 1;
            labels_num = 0;
            printf("{\n  \"data\": [\n");
        }

        /VGS/{
            labels_str = substr($0, index($0,$2));
            labels_num = split(labels_str,labels,"\t");
        }

        /^[0-9]/ {
            if (isFirstLine == 1) { 
                isFirstLine = 0;
                printf("    ");
            } else {
                printf("   ,");
            }
            printf("{\n");

            i=0;
            for ( i=1; i<labels_num; i++) {
                printf( "      \"" labels[i] "\":" $i ",\n");
            }
            printf( "      \"" labels[i] "\":" $i "\n");

            printf("    }\n");
        }

        END {
            print("] }");
        }
    ' > $pareto.json

gen=`echo $(grep "g s" ../M9DSE.log | wc -l) - 1 | bc`
echo Generation  $gen > status.txt
tac ../screenlog.0 | awk '/simulation/{print " (" $4, $5 , "of", $7 ")"; exit}' >> status.txt
