#!/bin/zsh
set -e
set -x
function die {
	echo "$1" >&2
	exit 1
}
./make.sh

for file in /yotta/pbwt/sparse /yotta/pbwt/subsampled; do
	[[ -r "$file.txt" ]] || die "cannot read $file.txt"


	[[ -r "$file".tv.txt ]] || ./convert.x "$file".txt > "$file".tv.txt

	for maxcols in 1000 2000 3000 4000 5000 6000 7000; do
		maxcolfile="$file.$maxcols"
		./travis.x "$maxcols" < "$file".tv.txt >! "$maxcolfile".grammar.txt

		
		cargo run --manifest-path=transform_grammar/Cargo.toml < "$maxcolfile".grammar.txt  > "$maxcolfile".grammar.bin

			# PlainSlp_32Fblc PlainSlp_FblcFblc PlainSlp_IblcFblc SelfShapedSlpV2_SdSd_Sd SelfShapedSlp_SdSd_Mcl SelfShapedSlp_SdSd_Sd ShapedSlpV2_Sd_SdMcl ShapedSlp_SdMclSd_SdMcl ShapedSlp_SdSdSd_SdMcl ShapedSlp_Status_SdMclSd_SdMcl; do
		for i in ShapedSlpV2_Sd_SdMcl; do 
			/home/niki/code/shapedSlp/build/SlpEncBuild -i "$maxcolfile".grammar.bin -f solca -e "$i" -o "$maxcolfile".grammar.slp.$i
		done
	done
done

