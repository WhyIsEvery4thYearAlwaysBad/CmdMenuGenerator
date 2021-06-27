#!/bin/sh
# Tests the program to ensure it works properly. Text colored in red indicates that something has gone wrong, in case you haven't realized.
# Script is POSIX-only for portability and enjoyment reasons.
TEST_FAIL_COUNT=0 # Amount of tests failed.
LAUNCH_OPS="${@}"
# Config
EXEC="../cmg-x64"
TEST_DIR="../tests"
# Func defines.

# Prints a message in bold red.
# $2 - If set prints Code number.
report_error() {
	printf "\033[1m\033[91m%s\033[0m%s\n" "${1?Cannot be blank}" "${2:+" (Code $2)"}"
	TEST_FAIL_COUNT=$((TEST_FAIL_COUNT+1))
}

# Wrapper for the cmenu generator.
exec_cmg() {
	case "$LAUNCH_OPS" in
	*-v* ) $EXEC ${@};;
	*--verbose--* ) $EXEC ${@};;
	* ) $EXEC ${@} > /dev/null;;
	esac
}

# Script
BINDIR=$(dirname "$(readlink -fn "$0")")
cd "${BINDIR}" || exit
case "$LAUNCH_OPS" in
	*--help* ) printf "Launch Options:\n\t-h, --help	Displays this menu.\n\t-v, --verbose	Desilences generator output.\n"; exit ;;
	*-h* ) printf "Launch Options:\n\t-h, --help	Displays this menu.\n\t-v, --verbose	Desilences generator output.\n"; exit ;;
esac
cd $TEST_DIR || exit
# Test the compiler itself first. 
## Valid CMenu file should be perfectly compilable.
printf "\0--------------------------------------\nCompiling valid cmenu. Should succeed.\n--------------------------------------\n"
exec_cmg ./valid-cmenu.txt
STATUS=$?
if [ $STATUS -eq 0 ]
then
	printf "\033[92mCompiler succeeded!\033[0m\n"
elif [ $STATUS -eq 255 ]
then
	report_error "Compiler ran into errors and exited normally!" "$STATUS"
else
	report_error "Compiler ran into errors and exited abnormally!" "$STATUS"
fi
rm -rf customvoicemenu
printf "\n-------------------------------------------------\nTesting error cases.\n-------------------------------------------------\n" 
for ErrorCMenuTestFile in $(dir ./error-tests)
do
	ErrorTypeMessage=$(grep -E "//\s*MetaInfo:" "./error-tests/$ErrorCMenuTestFile")
	ErrorTypeMessage=${ErrorTypeMessage#"// MetaInfo:"}
	exec_cmg "./error-tests/$ErrorCMenuTestFile"
	STATUS=$?
	printf "\033[1m%s\033[0m: " "$ErrorTypeMessage"
	if [ $STATUS -eq 255 ]
	then
		printf "\033[92mExpected Failure!\033[0m (Code %u)\n" "$STATUS"
	elif [ $STATUS -eq 0 ]
	then
		report_error "Succeeded! Not good." "$STATUS"
	else
		report_error "Unexpected Failure!" "$STATUS"
	fi
done
rm -rf customvoicemenu
# Test command line options.
## --help and -h launch options should succeed and then exit.
printf "\n---------------\nLaunch options.\n---------------\n"
for LaunchOP in "--help" "-h"
do
	exec_cmg --help
	STATE=$?
	printf "\033[1m%s\033[0m: " "$LaunchOP"
	if [ $STATE -eq 0 ]
	then
		printf "\033[92mSucceeded and exited as expected. \033[0m(Code %u)\n" "$STATUS"
	elif [ $STATE -eq 255 ]
	then
		report_error "$LaunchOP launch option failed." "$STATUS"
	else
		report_error "$LaunchOP launch option failed abnormally." "$STATUS"
	fi
done
## --output-dir
### with no param should fail and exit.
exec_cmg ./valid-cmenu.txt --output-dir
STATE=$?
printf "\033[1m--output-dir\033[0m: "
if [ $STATE -eq 255 ]
then
	printf "\033[92mFailed as expected. \033[0m(Code %u)\n" "$STATUS"
elif [ $STATE -eq 0 ]
then
	report_error "Succeeded and continued." "$STATUS"
else
	report_error "Failed abnormally." "$STATUS"
fi
### --output-dir with param should succeed.
exec_cmg ./valid-cmenu.txt --output-dir out-test-dir
STATE=$?
printf "\033[1m--output-dir (with parameter out-dest-dir)\033[0m: "
if [ $STATE -eq 0 ]
then
	printf "\033[92mSucceeded and continued. \033[0m(Code %s)\n" "$STATE"

elif [ $STATE -eq 255 ]
then
	report_error "Failed normally." "$EXEC" "$STATUS"
else
	report_error "Failed abnormally." "$EXEC" "$STATUS"
fi
rm -rf out-test-dir
# Check captions are being generated correctly.
printf "\n----------------------------\nValidating caption displays.\n----------------------------\n"
exec_cmg ./caption-cmenu.txt
STATUS=$?
if [ $STATUS -eq 0 ]
then
	printf "\033[92mCompiler succeeded!\033[0m (Code $STATUS)\n"
else
	report_error "Compiler ran into errors." "$STATUS"
fi
cd ./customvoicemenu || exit
## Verify that scripts actually exist before testing them.
if [ -f "./cfg/cmenu_initialize.cfg" ]
then
	printf "\033[92mCMenu init was generated.\033[0m\n"
else
	report_error "CMenu init was not generated!" "$STATUS"
fi
## Verify captions exist before testing them.
if [ -f "./resource/closecaption_commandmenu.txt" ]
then
	## Are captions encoded in UCS-2 (the required encoding for Source Engine captions)?
	FILE_ENCODING=$(file -bi ./resource/closecaption_commandmenu.txt | awk -F'=' '{print $2 }')
	if [ "$FILE_ENCODING" = "utf-16le" ]
	then
		printf "\033[92mCaption file is encoded in %s (aka UCS-2).\033[0m\n" "$(echo "$FILE_ENCODING" | tr '[:lower:]' '[:upper:]')"
	else
		report_error "Caption file is encoded incorrectly! (Encoding: $(echo $FILE_ENCODING | tr '[:lower:]' '[:upper:]'))"
	fi
	### Convert file to UTF-8 temporarily so that grep can properly process stuff.
	iconv -c -f UTF-16LE -t UTF-8 ./resource/closecaption_commandmenu.txt -o ./resource/temp_caption.txt --verbose
	## TOFINISH: Validate that the cfg file works.
	for CFGFile in $(dir ./cfg --ignore="cmenu_initialize.cfg")
	do
		CMenuRName=${CFGFile%.cfg}
		CMenuRName=${CMenuRName#\$cmenu_}
		### Ensure that captions are properly shown when the cmenu cfg is executed.
		MATCHES=$(grep -c -E "cc_emit\s*\_#cmenu.$CMenuRName" ./cfg/"$CFGFile")
		POSIXLY_CORRECT="true" grep --text "_#cmenu.$CMenuRName" ./resource/temp_caption.txt > /dev/null
		if [ "$MATCHES" -eq 0 ]
		then
			report_error "\033[91mCaptions for cmenu \"$CMenuRName\" aren't shown when running its cfg file!\n"
		elif [ "$MATCHES" -gt 1 ] 
		then
			report_error "Captions for cmenu \"$CMenuRName\" are shown $MATCHES times when running its cfg file!"
		fi
		### Verify the cmenu's captions are actually in the caption file.

	done
else
	report_error "Caption file has not been generated, when it should have been!"
fi
cd .. || exit
rm -rf ./customvoicemenu
# End Testing
if [ $TEST_FAIL_COUNT -le 0 ]
then
	printf "\033[92mNo major issues detected!\033[0m\n"
else
	report_error "$TEST_FAIL_COUNT failures detected! Patch the bugs!"
fi
exit