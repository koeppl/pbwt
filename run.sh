#!/bin/zsh
set -e
set -x
function die {
	echo "$1" >&2
	exit 1
}

function task {
	echo "Threshold: $threshold"
	# ./pbwt.x -t "$threshold" -i /yotta/pbwt/durbin.txt
	./pbwt.x -i /yotta/pbwt/durbin.txt -t "$threshold" -d $file.$threshold.divarray -h $file.$threshold.grammar.txt -m $file.$threshold.matrix -n $file.$threshold.intervals
	#-w 2000
	~/code/bigrepair/bigrepair $file.$threshold.matrix
	~/code/shapedSlp/build/SlpEncBuild -i $file.$threshold.matrix -f Bigrepair -e ShapedSlpV2_Sd_SdMcl -o $file.$threshold.matrix.slp
	~/code/bigrepair/bigrepair -i $file.$threshold.divarray
	# ~/code/shapedSlp/build/SlpEncBuild -i $file.$threshold.divarray -f Bigrepair -e ShapedSlpV2_Sd_SdMcl -o $file.$threshold.divarray.slp
	cargo run --manifest-path=transform_grammar/Cargo.toml -- -i $file.$threshold.grammar.txt  -o $file.$threshold.grammar.bin
	~/code/shapedSlp/build/SlpEncBuild -i $file.$threshold.grammar.bin -f solca -e ShapedSlpV2_Sd_SdMcl -o $file.$threshold.grammar.slp
}


make

file=/yotta/pbwt/durbin.txt
mkdir -p log

for threshold in 0.1 0.08 0.05 0.03 0.01; do
		task 2>&1 | tee log/durbin.$threshold.log  &
done
wait



exit 0
./make.sh

for file in /yotta/pbwt/sparse /yotta/pbwt/subsampled; do
	[[ -r "$file.txt" ]] || die "cannot read $file.txt"


	[[ -r "$file".tv.txt ]] || ./convert.x "$file".txt > "$file".tv.txt

	for maxcols in 1000 2000 3000 4000 5000 6000 7000; do
		maxcolfile="$file.$maxcols"
		./travis.x "$maxcols" < "$file".tv.txt >! "$maxcolfile".grammar.txt

		

			# PlainSlp_32Fblc PlainSlp_FblcFblc PlainSlp_IblcFblc SelfShapedSlpV2_SdSd_Sd SelfShapedSlp_SdSd_Mcl SelfShapedSlp_SdSd_Sd ShapedSlpV2_Sd_SdMcl ShapedSlp_SdMclSd_SdMcl ShapedSlp_SdSdSd_SdMcl ShapedSlp_Status_SdMclSd_SdMcl; do
		for i in ShapedSlpV2_Sd_SdMcl; do 
			/home/niki/code/shapedSlp/build/SlpEncBuild -i "$maxcolfile".grammar.bin -f solca -e "$i" -o "$maxcolfile".grammar.slp.$i
		done
	done
done

