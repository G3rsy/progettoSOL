#!/bin/bash
exec 3<logClient

i=0

while read -e -u 3 -a line; do
	if [ ${#line[@]} == "4" ]; then
		id[$i]=${line[1]}
		secret[$i]=${line[3]}

		i=$(($i + 1))
	fi
done
#
#FIN QUI: ho parsato il logClient e recuperato id e secret di ogni client
#


#
#ORA:parso il logSuper per confrontare le stime del supervisor
#
exec 4<logSuper
dim=${#id[@]}
i=0
while read -e -u 4 -a line; do

	#se la stringa e' del supervisor, cerco indice dell'id
	if (( ${#line[@]} == 8 ));then
		#parte la ricerca dell'id

		while (( "$i" < ${#id[@]} )) && ! [ "${id[$i]}" = "${line[4]}" ];do
			i=$(($i + 1))
		done

		#echo "all'indice $i,  trovo l'id giusto e inserisco ${line[2]}"

		#lo trovo e creo un array che contiene le stime del supervisor al giusto indice
		if (( "$i" < ${#id[@]} ));then
			supTime[$i]=${line[2]}
		fi

	fi
	i=0
done

neg=-1
totError=0
correctErr=0

for ((i=0; i<${#id[@]}; i+=1)); do
	error[$i]=$((${secret[$i]} - ${supTime[$i]}))
	
	if ((${error[$i]} < 0));then
		error[$i]=$((${error[$i]} * neg))
	fi

	echo "id - ${id[$i]}__secret - ${secret[$i]}__stima - ${supTime[$i]}"

	if ((${error[$i]} < 25)); then
		correctErr=$(($correctErr + 1))
	fi

	totError=$(($totError + ${error[$i]}))
done

midError=$(($totError / ${#id[@]}))
echo "CORRETTE: $correctErr"
echo "ERRORE MEDIO: $totError / ${#id[@]} = $midError"

