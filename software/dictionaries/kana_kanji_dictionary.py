import xml.etree.ElementTree as ET
import pandas as pd
import itertools
import mmh3

def ranges(i):
    for a, b in itertools.groupby(enumerate(i), lambda pair: pair[1] - pair[0]):
        b = list(b)
        yield b[0][1], b[-1][1]

def bits(x):
    data = []
    for i in range(8):
        data.insert(0, int((x & 1) == 1))
        x = x >> 1
    return data

def reverse_Bits(n, no_of_bits):
    result = 0
    for i in range(no_of_bits):
        result <<= 1
        result |= n & 1
        n >>= 1
    return result

def print_balanced_bst(sList, prefix, outFile):
    # If the list is empty, return an empty string (null pointer).
    if not sList:
        return 0
    
    # Length of list.
    n = len(sList)
    # Midpoint of list.
    mid = n // 2
    # The key for the node is the middle element of the list.
    key = sList[mid]
    # The left (lower) list consists of list elements to left of key.
    lList = sList[0:mid]
    # The right (higher) list consists of list elements to right of key.
    rList = sList[mid+1:n]
    # Recurse on balanced BST for the left and right lists.
    lNode = print_balanced_bst(lList, prefix, outFile)
    rNode = print_balanced_bst(rList, prefix, outFile)
    
    # Print the C structure representing the node.
    # Data Type 4?: Node for balanced binary tree.
    # typedef struct bbt_node {
    #   const uint64_t key;
    #   const reading_md* onmd;
    #   const bbt_node * const lnode;
    #   const bbt_node * const rnode;
    # } bbt_node;
    lCurl = '{'
    rCurl = '}'
    print(f"const bbt_node {prefix}_node{key} {lCurl}", end='', file=outFile)
    print(f"{key}, &{prefix}_md{key},", end='', file=outFile)
    if lNode != 0:
        print(f" &{prefix}_node{lNode},", end='', file=outFile)
    else:
        print(" NULL,", end='', file=outFile)

    if rNode != 0:
        print(f" &{prefix}_node{rNode}", end='', file=outFile)
    else:
        print(" NULL", end='', file=outFile)

    print(f"{rCurl};", file=outFile)
    return key

# List of Kanji that are more frequent in Kanji lists othen than
# Kanji Dictionary 2.  Removed '𠮟', as it is very rarely used.
altKanji = ['昧', '碗', '潰', '柵', '璧', '賦', '頁', '覗', '廻', '箇',
            '彙', '窟', '旦', '拶', '云', '塞', '曖', '勿', '嘩', '茸',
            '萎', '紐', '繋', '籠', '痺', '餃', '飴', '戚', '隙',
            '且', '丙', '伎', '侶', '傲', '僅', '冶', '刹', '剝', '勃',
            '勾', '吏', '咽', '唾', '喩', '嘲', '塑', '塡', '墾', '妬',
            '宛', '巾', '弄', '弐', '慄', '憬', '拉', '捗', '捻', '斤',
            '朕', '桁', '梗', '楷', '毀', '氾', '汰', '沃', '淫', '溺',
            '濫', '爵', '璃', '璽', '痘', '痩', '瘍', '瞭', '箋', '綻',
            '緻', '羞', '羨', '耗', '肘', '腺', '臆', '舷', '苛', '蔽',
            '虞', '詣', '詮', '諧', '謁', '貌', '貪', '賂', '踪', '辣',
            '遜', '遡', '遵', '酎', '醒', '采', '錮', '頓', '頰', '顎',
            '骸', '鬱']

# Load in the Novel 5K CSV file.
dfN5K = pd.read_csv('./Novel_5K.csv')
# Add a column indicating match with KanjiDict 2.
dfN5K['Matched'] = 0

print("Extracting usable kanji from kanjidic2.xml....")
# Read in the XML file containing Kanji Dictionary 2.
tree = ET.parse('./kanjidic2.xml')
# Reference the root of the tree.
root = tree.getroot()

# Create a new dataframe to hold the usable kanji entries.
# Usable kanji must have some reading or nanori.
df1 = pd.DataFrame(columns = ['UCS', 'Kanji', 'KD2_Rank', 'N5K_Rank', 'Meanings', 'OnReadings', 'KunReadings', 'Nanori'])

for character in root.findall('character'):
    # Extract the Unicode value for the Kanji character.
    ucs = character.find("codepoint/cp_value/[@cp_type='ucs']").text
    # Extract the Kanji character itself.
    kanji = character.find("literal").text
    # Attempt to extract the frequency ranking value.
    freq_tag = character.find("misc/freq")
    # Assign arbitrary frequency rank if no ranking is found.
    if freq_tag is not None:
        freq = int(freq_tag.text)
    elif kanji in altKanji:
        freq = 8000
    else:
        freq = 10000

    # Find Novel 5K ranking for the Kanji character.
    ndxList = dfN5K.index[dfN5K['Kanji'] == kanji].tolist()
    if len(ndxList) != 0:
        # The Kanji is present in Novel 5K and there should be only one instance.
        ndx = ndxList[0]
        # Obtain Novel 5K frequency ranking value.
        n5kRank = dfN5K.iloc[ndx]['Source Order']
        dfN5K.at[ndx, 'Matched'] = dfN5K.at[ndx, 'Matched'] + 1
    else:
        # The Kanji is not present Novel 5K data set, so use a default value.
        # print(f'Kanji {kanji} is not found in Novel 5K.')
        n5kRank = 10000

    if freq < 10000 and n5kRank == 10000:
        n5kRank = 9000
        print(f'Kanji: {kanji} ({ucs}) is in Kanji Dict 2 ({freq}) but not in Novel 5K ({n5kRank})!')
        # Preserve KanjiDict2 ranking if low enough.
        if freq < 8000:
            n5kRank = freq

    # Construct list of meanings.
    meanings = []
    for meaning in character.findall("reading_meaning/rmgroup/meaning"):
        if meaning.attrib == {}:
            meanings.append(meaning.text)

    meanings = ', '.join(meanings)

    # Construct list of onyomi.
    on_list = []
    # Test if r_type='ja_on' exists before iterating.
    for on_item in character.findall("reading_meaning/rmgroup/reading/[@r_type='ja_on']"):
        on_list.append(on_item.text)
    
    # Construct list of kunyomi.
    kun_list = []
    # Test if r_type='ja_kun' exists before iterating.
    for kun_item in character.findall("reading_meaning/rmgroup/reading/[@r_type='ja_kun']"):
        kun_list.append(kun_item.text)
    
    # Construct list of nanori.
    nan_list = []
    # Test if an nanori exists before iterating.
    for nan_item in character.findall("reading_meaning/nanori"):
        nan_list.append(nan_item.text)

    # If there are no readings for this kanji, so skip further processing.
    if len(on_list) == 0 and len(kun_list) == 0 and len(nan_list) == 0:
        print(f"Unicode {ucs}, with frequency {freq}, has no readings or nanori!")
        continue

    # Construct and concatenate a new row to DataFrame 1.
    new_row = pd.DataFrame([[ucs, kanji, freq, n5kRank, meanings, on_list, kun_list, nan_list]],
                           columns=['UCS', 'Kanji', 'KD2_Rank', 'N5K_Rank', 'Meanings', 'OnReadings', 'KunReadings', 'Nanori'])
    df1 = pd.concat([df1, new_row], ignore_index=True)

print("Done extracting usable kanji from kanjidic2.xml.")

# Discard all Kanji with a rank of 10000, as they are too rare.
df1Subset = df1.loc[df1['N5K_Rank'] < 10000]

# Sort the subset according to the Novel 5K frequency ranking.  This is done
# to allow the file content to be reduced from the end of the file and still
# provide the most common Kanji.
df1Sort = df1Subset.sort_values(by='N5K_Rank').copy()

# This Unicode-ordered list of Kana and Kanji characters are used generate
# The font bitmap data for LVGL using the LGVL Online Font Converter
# (https://lvgl.io/tools/fontconverter).  The name of the font is KanaKanjiFontSmall.
# The size is 32 pixels with 1 bpp.  Font used is Noto_Sans_Mono_CJK_JP-Regular.otf.
# Sort the subset according to Kanji Unicode and generate an ordered list.
df1KanjiSort = df1Subset.sort_values(by='UCS').copy()
kanjiHexList = df1KanjiSort['UCS'].tolist()
# Covert ordered list to ranges to reduce size of list.
kanjiIntList = []
for kanjiHex in kanjiHexList:
    # Make string of Kana bytes look like a hex number.
    kanjiHex = "0x" + kanjiHex
    # Convert hex to interger value for sorting purposes.
    kanjiIntList.append(int(kanjiHex, 16))

# Convert the ranges to a Python list
kanjiRangeList = list(ranges(kanjiIntList))

# Write out the list, with Hiragana and Katakana ranges first,
# followed by the Kanji ranges.  Ranges that have only one
# Kanji Unicode are specified a single Kanji Unicode.
with open("./kana_kanji_subset.txt", "w") as f:
    print("0x2B06, 0x2B07, 0x3005, 0x3041-0x3096, 0x309D-0x309E", end='', file=f)
    print(", 0x30A1-0x30F7, 0x30FC-0x30FE", end='', file=f)
    for subrange in kanjiRangeList:
        if subrange[0] == subrange[1]:
            startRange = "{:04x}".format(subrange[0])
            print(f", 0x{startRange}", end='', file=f)
        else:
            startRange = "{:04x}".format(subrange[0])
            endRange = "{:04x}".format(subrange[1])
            print(f", 0x{startRange}-0x{endRange}", end='', file=f)

# Print out the meaning strings for each Kanji, ordered by Novel 5K
# frequency ranking, to a file.
struct_data = []
# Open Kanji meaning string file for appended writing.
with open("./kanji_ms.h", "w") as f:
    # Iterate on the rows of he sorted dataframe.
    for idx, row in df1Sort.iterrows():
        # The Unicode character is the unique part of each bitmap name.
        ucs_name = str.upper(row['UCS'])
        # Add bitmap name and Novel 5K rank to structure data.
        struct_data.append([ucs_name, row['N5K_Rank']])
        # Data structure 2: Kanji meaning string.
        # const char kanji_ms<UCS>[] = "<Kanji UCS meaning string>";
        print(f"const char kanji_ms{ucs_name}[] = \"{row['Meanings']}\";", file=f)

f.close()

# Open Kanji metadata file for appended writing.  Now the metadata only needs
# to convey the Unicode value, the frequency rank, and meaning.  The actual
# font bitmaps are obtained from KanaKanjiFontSmall, using the utf8 Unicode
# value as the character selector.
with open("./kanji_md.h", "w") as f:
    # Data Type 1: Kanji metadata.
    # typedef struct kanji_md {
    #     const uint16_t unicode;    // Kanji unicode
    #     const uint16_t rank;       // Kanji frequency rank
    #     const char* const meaning; // Meanings for the Kanji
    # };
    # Print the structure definition.
    print("#ifndef KANJI_METADATA_TYPE", file=f)
    print("#define KANJI_METADATA_TYPE", file=f)
    print("typedef struct kanji_md {", file=f)
    print("  const uint16_t unicode;", file=f)
    print("  const uint16_t rank;", file=f)
    print("  const char* const meaning;", file=f)
    print("} kanji_md;", file=f)
    print("#endif\n", file=f)

    # Iterate on the elements of the structure data array.
    # [ucs_name, row['N5K_Rank']]
    open_curl = "{"
    close_curl = "}"
    for item in struct_data:
        ucs = item[0]
        rank = item[1]
        print(f"const kanji_md kanji_md{ucs} = ", end='', file=f)
        print(f"{open_curl}0x{ucs}, {rank}, ", end='', file=f)
        print(f"kanji_ms{ucs}{close_curl};", file=f)

f.close()

print("Building onyomi-to-kanji cross refrence")
df2 = pd.DataFrame(columns = ['Onyomi', 'affixList', 'freqList', 'UCSList'])

print("Building kunyomi-to-kanji cross refrence")
df3 = pd.DataFrame(columns = ['Kunyomi', 'okuriList', 'affixList', 'freqList', 'UCSList'])

print("Building nanori-to-kanji cross refrence")
df4 = pd.DataFrame(columns = ['Nanori', 'okuriList', 'freqList', 'UCSList'])

# Process each row of DataFrame 1 
for df1_ndx, row in df1Subset.iterrows():
    # Iterate on each onyomi for this row.
    for onyomi in row['OnReadings']:
        # Detect presence of leading hyphen - yes, there are a few present in onyomi.
        # Remember the presence of the suffix before the strip.
        if onyomi.startswith('-'):
            affix = 'suffix'
        else:
            affix = 'none'

        # Parse out hyphens - yes, there are a few present in onyomi.
        onyomi = onyomi.strip('-')

        # Attempt to find the onyomi in DataFrame 2.
        ndxList = df2.index[df2['Onyomi'] == onyomi].tolist()
        if len(ndxList) != 0:
            # The onyomi is present and there should be only one instance.
            ndx = ndxList[0]
            # Add the suffix flag to list for the onyomi.
            df2.iloc[ndx]['affixList'].append(affix)
            # Add frequency ranking value to list for the onyomi.
            df2.iloc[ndx]['freqList'].append(row['N5K_Rank'])
            # Add unicode value to list for the onyomi.
            df2.iloc[ndx]['UCSList'].append(row['UCS'])
        else:
            # The onyomi is not present in DataFrame 2, so add it, as well
            # as the frequency ranking and unicode values.
            new_row = pd.DataFrame([[onyomi, [affix], [row['N5K_Rank']], [row['UCS']]]],
                                   columns=['Onyomi', 'affixList','freqList', 'UCSList'])
            # Add the new row into DataFrame 2.
            df2 = pd.concat([df2, new_row], ignore_index=True)
            
    # Iterate on each kunyomi for this row.
    for kunyomi in row['KunReadings']:
        # Parse out hyphens and okurigana.
        # CHECK THIS FOR CORRECTNESS!!!!
        if kunyomi.startswith('-'):
            affix = 'suffix'
        elif kunyomi.endswith('-'):
            affix = 'prefix'
        else:
            affix = 'none'
        
        kunyomi = kunyomi.strip('-')
        kunyomi = kunyomi.split('.')
        if len(kunyomi) == 1:
            okurigana = ''
        elif len(kunyomi) == 2:
            okurigana = kunyomi[1]
        else:
            print('Too many dots!')

        kunyomi = kunyomi[0]
        # Attempt to find the kunyomi in DataFrame 3.
        ndxList = df3.index[df3['Kunyomi'] == kunyomi].tolist()
        if len(ndxList) != 0:
            # The kunyomi is present and there should be only one instance.
            ndx = ndxList[0]
            # Determine whether unicode is already in the list.
            if row['UCS'] not in df3.iloc[ndx]['UCSList']:
                # Add okurigana value as a list to list for the kunyomi.
                df3.iloc[ndx]['okuriList'].append(okurigana)
                # Add the affix flag as a list to list for the kunyomi.
                df3.iloc[ndx]['affixList'].append(affix)
                # Add frequency ranking value to list for the kunyomi.
                df3.iloc[ndx]['freqList'].append(row['N5K_Rank'])
                # Add unicode value to list for the kunyomi.
                df3.iloc[ndx]['UCSList'].append(row['UCS'])
            else:
                # The unicode is already there for this kunyomi, so...
                #print('Duplicate kunyomi for the same kanji character')
                #print('Make a sublist of okurigana for that kunyomi')
                # Find index of the unicode that is already in the list.
                uNdx = df3.iloc[ndx]['UCSList'].index(row['UCS'])
                # Append the new okurigana to the indexed list in the okurigana list.
                oList = df3.iloc[ndx]['okuriList'][uNdx]
                if type(oList) is not list:
                    df3.iloc[ndx]['okuriList'][uNdx] = [oList, okurigana]
                else:
                    oList.append(okurigana)
                    df3.iloc[ndx]['okuriList'][uNdx] = oList

                # Append the new affix to the indexed list in the affix list.
                aList = df3.iloc[ndx]['affixList'][uNdx]
                if type(aList) is not list:
                    df3.iloc[ndx]['affixList'][uNdx] = [aList, affix]
                else:
                    aList.append(affix)
                    df3.iloc[ndx]['affixList'][uNdx] = aList

        else:
            # The onyomi is not present in DataFrame 3, so add it, as well
            # as the okurigana, frequency ranking, and unicode values.
            new_row = pd.DataFrame([[kunyomi, [okurigana], [affix], [row['N5K_Rank']], [row['UCS']]]],
                                   columns=['Kunyomi', 'okuriList', 'affixList', 'freqList', 'UCSList'])
            # Add the new row into DataFrame 3.
            df3 = pd.concat([df3, new_row], ignore_index=True)

    # Iterate on each nanori for this row.
    for nanori in row['Nanori']:
        # Attempt to find the nanori in DataFrame 4.

        nanori = nanori.split('.')
        if len(nanori) == 1:
            okurigana = ''
        elif len(nanori) == 2:
            okurigana = nanori[1]
        else:
            print('Too many dots!')

        nanori = nanori[0]
        ndxList = df4.index[df4['Nanori'] == nanori].tolist()
        if len(ndxList) != 0:
            # The nanori is present and there should be only one instance.
            ndx = ndxList[0]
            # Add okurigana value to list for the kunyomi.
            df4.iloc[ndx]['okuriList'].append(okurigana)
            # Add frequency ranking value to list for the nanori.
            df4.iloc[ndx]['freqList'].append(row['N5K_Rank'])
            # Add unicode value to list for the nanori.
            df4.iloc[ndx]['UCSList'].append(row['UCS'])
        else:
            # The nanori is not present in DataFrame 4, so add it, as well
            # as the frequency ranking and unicode values.
            new_row = pd.DataFrame([[nanori, [okurigana], [row['N5K_Rank']], [row['UCS']]]],
                                   columns=['Nanori', 'okuriList', 'freqList', 'UCSList'])
            # Add the new row into DataFrame 4.
            df4 = pd.concat([df4, new_row], ignore_index=True)

# Sort frequency and unicode lists of DataFrame 2 with frequency list acting as key.
for df2_ndx, row in df2.iterrows():
    list1, list2, list3 = (list(t) for t in zip(*sorted(zip(row['freqList'], row['affixList'], row['UCSList']))))
    df2.iloc[df2_ndx]['freqList'] = list1
    df2.iloc[df2_ndx]['affixList'] = list2
    df2.iloc[df2_ndx]['UCSList'] = list3

# Sort DataFrame 2 so that onyomi are in acending alphabetical order.
df2Sort = df2.sort_values('Onyomi', ignore_index=True)

# Sort frequency, okurigana, and unicode lists of DataFrame 3 with frequency list acting as key.
for df3_ndx, row in df3.iterrows():
    #list1, list2, list3, list4 = (list(t) for t in zip(*sorted(zip(row['freqList'], row['okuriList'], row['affixList'], row['UCSList']))))
    pickList = sorted(range(len(row['freqList'])), key=lambda k: row['freqList'][k])
    list1 = []
    list2 = []
    list3 = []
    list4 = []
    for ndx in pickList:
        list1.append(row['freqList'][ndx])
        list2.append(row['okuriList'][ndx])
        list3.append(row['affixList'][ndx])
        list4.append(row['UCSList'][ndx])

    df3.iloc[df3_ndx]['freqList'] = list1
    df3.iloc[df3_ndx]['okuriList'] = list2
    df3.iloc[df3_ndx]['affixList'] = list3
    df3.iloc[df3_ndx]['UCSList'] = list4

# Sort DataFrame 3 so that kunyomi are in acending alphabetical order.
df3Sort = df3.sort_values('Kunyomi', ignore_index=True)

# Sort frequency and unicode lists of DataFrame 4 with frequency list acting as key.
for df4_ndx, row in df4.iterrows():
    list1, list2, list3 = (list(t) for t in zip(*sorted(zip(row['freqList'], row['okuriList'], row['UCSList']))))
    df4.iloc[df4_ndx]['freqList'] = list1
    df4.iloc[df4_ndx]['okuriList'] = list2
    df4.iloc[df4_ndx]['UCSList'] = list3
    #print(f"Row {df4_ndx} {df4.iloc[df4_ndx]['Nanori']} {df4.iloc[df4_ndx]['freqList']} {df4.iloc[df4_ndx]['UCSList']}")

# Sort DataFrame 4 so that nanori are in acending alphabetical order.
df4Sort = df4.sort_values('Nanori', ignore_index=True)

# Load Kana character to hex map as an associative array.  The Kana (column 0)
# is treated as the index and there is no header.
kanaMap = pd.read_csv('./kana_list.csv', index_col=0, header=None)
# Convert the DataFrame into a Series.
kanaMap = kanaMap.squeeze()

# START OF ONYOMI OUTPUT PROCESSING
# Add column to hold integer value that is the decimal representation of the
# concatenated lower bytes of the Katakana characters of the Onyomi string.
# After all rows are processed, none should contain the default value.
df2Sort['kanaBytes'] = ""
df2Sort['KanaInt'] = 0

# Enumeration for describing enhancements to onyomi, kunyomi, and
# nanori readings, and jukujikun.
# enum affix_enum {
#   none,   // No enhancments to reading (e.g., xyz).
#   prefix, // Reading is a prefix (e.g., wxyz-).
#   suffix, // Reading is a suffix (e.g., -wxyz).
#   jword,  // Japanese word string with jukujikun or irregular reading.
#   meaning // English meaning string for Japanese word enumerated above.
# };

# Data Type 2: Structure typedef of metadata for Okurigana (and other readings)
# typedef struct okuri_md {
#   const uint8_t len;      // Number of enumerations/character arrays.
#   const affix_enum *alist; // Pointer ot array of enumerations, one for each character array.
#   const char **clist;      // Pointer to array of character arrays, for for each enumeration.
# } okuri_md;

# Data Type 3: Structure typedef of metadata for Reading
# typedef struct reading_md {
#   const uint16_t len;             // Number of Okurigana and Kanji structures.
#   const okuri_md * const * olist; // Pointer to array of okurigana metadata structures.
#   const kanji_md * const * klist; // Pointer to array of kanji metadata structures.
# } reading_md;
# Data structure 3:  Array of Kanji metadata structures referenced by a
# specific onyomi, kunyomi, or nanori.  The order of the Kanji metadata
# structures in the list is the ascending order of Kanji frequency rank
# The value of XXXX is the numeric represenation of the kana for the
# onyomi, kunyomi, or nanori.
# const kanji_md const kmXXXX[] = {&kanji_md<UCS1>, &kanji_md<UCS2>, ... &kanji_md<UCSn>};
open_curl = "{"
close_curl = "}"
with open("onyomi.h", "w") as f:
    # The Onyomi data does not contain any okurigana, but there are Onyomi some that are
    # suffixes. This requires creating and initializing okurigana metadata structure arrays.
    print("/* onyomi.h */\n", file=f)
    print("#ifndef AFFIX_ENUM", file=f)
    print("#define AFFIX_ENUM", file=f)
    print("enum affix_enum {", file=f)
    print("  none,   // No enhancments to reading (e.g., xyz).", file=f)
    print("  prefix, // Reading is a prefix (e.g., wxyz-).", file=f)
    print("  suffix, // Reading is a suffix (e.g., -wxyz).", file=f)
    print("  jword,  // Japanese word string with jukujikun or irregular reading.", file=f)
    print("  meaning // English meaning string for Japanese word enumerated above.", file=f)
    print("};", file=f)
    print("#endif\n", file=f)

    print("#ifndef OKURI_METADATA_TYPE", file=f)
    print("#define OKURI_METADATA_TYPE", file=f)
    print("typedef struct okuri_md {", file=f)
    print("  const uint8_t len;", file=f)
    print("  const affix_enum * const alist;", file=f)
    print("  const char **clist;", file=f)
    print("} okuri_md;", file=f)
    print("#endif\n", file=f)

    print("#ifndef READING_METADATA_TYPE", file=f)
    print("#define READING_METADATA_TYPE", file=f)
    print("typedef struct reading_md {", file=f)
    print("  const uint16_t len;", file=f)
    print("  const okuri_md * const * olist;", file=f)
    print("  const kanji_md * const * klist;", file=f)
    print("} reading_md;", file=f)
    print("#endif", file=f)

    # Iterate on each onyomi to create an array of kanji metadata for each onyomi.
    for df2_ndx, row in df2Sort.iterrows():
        # Convert syllables of Kana into a number string for naming and
        # referencing of structure array.
        kanaBytes = ""
        for kana in row['Onyomi']:
            kanaBytes = kanaBytes + kanaMap[kana]

        # Convert to a byte string.
        kanaBytesStr = bytes.fromhex(kanaBytes)
        # Generate 32-bit hash
        kanaInt = mmh3.hash(kanaBytesStr, signed=False)
        kanaBytes = hex(kanaInt)
        # Fill KanaBytes and KanaInt columns for the row.
        df2Sort.at[df2_ndx, 'KanaBytes'] = kanaBytes
        df2Sort.at[df2_ndx, 'KanaInt'] = kanaInt

        # For onyomi, there is only affix information. Either 'none' or 'suffix'.
        # There is always only one affix value for each onyomi reading, further
        # simplifying the generation script.
        allNoneList = all(ele == 'none' for ele in row['affixList'])
        if not allNoneList:
            sndx = 0
            for affix in row['affixList']:
                print(f"const affix_enum on_affix{kanaBytes}_{sndx:02d}[] = ", end='', file=f)
                print(f"{open_curl}{affix}{close_curl};", file=f)
                
                print(f"const okuri_md on_okuri_md{kanaBytes}_{sndx:02d} = ", end='', file=f)
                print(f"{open_curl}1, on_affix{kanaBytes}_{sndx:02}, NULL{close_curl};", file=f)

                sndx += 1
            
            print(f"const okuri_md * on_okuri{kanaBytes}[] = ", end='', file=f)
            delimit = '{'
            for ndx in range(sndx):
               print(f"{delimit}&on_okuri_md{kanaBytes}_{ndx:02}", end='', file=f)
               delimit = ', '

            print('};', file=f)

        delimit = '{'
        print(f"const kanji_md * const on_kanji{kanaBytes}[] = ", end='', file=f)
        for ucs in row['UCSList']:
            ucs_name = str.upper(ucs)
            print(f"{delimit}&kanji_md{ucs_name}", end='', file=f)
            delimit = ', '

        print('};', file=f)
        print(f"const reading_md onyomi_md{kanaBytes} = ", end='', file=f)
        if allNoneList:
            print(f"{open_curl}{len(row['UCSList'])}, NULL, on_kanji{kanaBytes}{close_curl};", file=f)
        else:
            print(f"{open_curl}{len(row['UCSList'])}, on_okuri{kanaBytes}, on_kanji{kanaBytes}{close_curl};", file=f)
    
    # Data type 3: Node for balanced binary tree.
    # typedef struct bbt_node {
    #   const uint32_t key;
    #   const reading_md* rmd;
    #   const bbt_node * const lnode;
    #   const bbt_node * const rnode;
    # };
    print("\n#ifndef BBT_NODE_TYPE", file=f)
    print("#define BBT_NODE_TYPE", file=f)
    print("typedef struct bbt_node {", file=f)
    print("  const uint32_t key;", file=f)
    print("  const reading_md* rmd;", file=f)
    print("  const bbt_node * const lnode;", file=f)
    print("  const bbt_node * const rnode;", file=f)
    print("} bbt_node;", file=f)
    print("#endif", file=f)

    # Pull out the KanaBytes and KanaInt colums from the sorted
    # onyomi DataFrame (df2Sort).
    df2Sub = df2Sort[['KanaBytes', 'KanaInt']]
    # Sort according to KanaInt.
    df2SubSort = df2Sub.sort_values('KanaInt', ignore_index=True)
    # Form a list from the sorted KanaBytes column.
    onyomiList = df2SubSort['KanaBytes'].tolist()
    # Generate the node data structures for the Balanced Binary Structure Tree.
    root_node = print_balanced_bst(onyomiList, "onyomi", f)
    # Generate a pointer to the root node.
    print(f"const bbt_node *onyomi_root_node = &onyomi_node{root_node};", file=f)

f.close()

# START OF KUNYOMI OUTPUT PROCESSING
# Add column to hold integer value that is the decimal representation of the
# concatenated lower bytes of the Katakana characters of the Onyomi string.
# After all rows are processed, none should contain the default value.
df3Sort['kanaBytes'] = ""
df3Sort['KanaInt'] = 0

# Enumeration for describing enhancements to onyomi, kunyomi, and
# nanori readings, and jukujikun.
# enum affix_enum {
#   none,   // No enhancments to reading (e.g., xyz).
#   prefix, // Reading is a prefix (e.g., wxyz-).
#   suffix, // Reading is a suffix (e.g., -wxyz).
#   jword,  // Japanese word string with jukujikun or irregular reading.
#   meaning // English meaning string for Japanese word enumerated above.
# };

# Data Type 2: Structure typedef of metadata for Okurigana (and other readings)
# typedef struct okuri_md {
#   const uint8_t len;      // Number of enumerations/character arrays.
#   const affix_enum *alist; // Pointer ot array of enumerations, one for each character array.
#   const char **clist;      // Pointer to array of character arrays, for for each enumeration.
# } okuri_md;

# Data Type 3: Structure typedef of metadata for Reading
# typedef struct reading_md {
#   const uint16_t len;             // Number of Okurigana and Kanji structures.
#   const okuri_md * const * olist; // Pointer to array of okurigana metadata structures.
#   const kanji_md * const * klist; // Pointer to array of kanji metadata structures.
# } reading_md;
# Data structure 3:  Array of Kanji metadata structures referenced by a
# specific onyomi, kunyomi, or nanori.  The order of the Kanji metadata
# structures in the list is the ascending order of Kanji frequency rank
# The value of XXXX is the numeric represenation of the kana for the
# onyomi, kunyomi, or nanori.
# const kanji_md const kmXXXX[] = {&kanji_md<UCS1>, &kanji_md<UCS2>, ... &kanji_md<UCSn>};
open_curl = "{"
close_curl = "}"
with open("kunyomi.h", "w") as f:
    print("/* kunyomi.h */\n", file=f)
    print("#ifndef AFFIX_ENUM", file=f)
    print("#define AFFIX_ENUM", file=f)
    print("enum affix_enum {", file=f)
    print("  none,   // No enhancments to reading (e.g., xyz).", file=f)
    print("  prefix, // Reading is a prefix (e.g., wxyz-).", file=f)
    print("  suffix, // Reading is a suffix (e.g., -wxyz).", file=f)
    print("  jword,  // Japanese word string with jukujikun or irregular reading.", file=f)
    print("  meaning // English meaning string for Japanese word enumerated above.", file=f)
    print("};", file=f)
    print("#endif\n", file=f)

    print("#ifndef OKURI_METADATA_TYPE", file=f)
    print("#define OKURI_METADATA_TYPE", file=f)
    print("typedef struct okuri_md {", file=f)
    print("  const uint8_t len;", file=f)
    print("  const affix_enum * const alist;", file=f)
    print("  const char **clist;", file=f)
    print("} okuri_md;", file=f)
    print("#endif\n", file=f)

    print("#ifndef READING_METADATA_TYPE", file=f)
    print("#define READING_METADATA_TYPE", file=f)
    print("typedef struct reading_md {", file=f)
    print("  const uint16_t len;", file=f)
    print("  const okuri_md * const * olist;", file=f)
    print("  const kanji_md * const * klist;", file=f)
    print("} reading_md;", file=f)
    print("#endif", file=f)

    # Iterate on each kunyomi to create an array of kanji metadata for each kunyomi.
    for df3_ndx, row in df3Sort.iterrows():
        # Convert syllables of Kana into a number string for naming and
        # referencing of structure array.
        kanaBytes = ""
        for kana in row['Kunyomi']:
            kanaBytes = kanaBytes + kanaMap[kana]

        # Convert to a byte string.
        kanaBytesStr = bytes.fromhex(kanaBytes)
        # Generate 32-bit hash
        kanaInt = mmh3.hash(kanaBytesStr, signed=False)
        kanaBytes = hex(kanaInt)
        # Fill KanaBytes and KanaInt columns for the row.
        df3Sort.at[df3_ndx, 'KanaBytes'] = kanaBytes
        df3Sort.at[df3_ndx, 'KanaInt'] = kanaInt

        # Determine whether all affix values are 'none'.  Assume true.
        allNoneList = True
        for ele in row['affixList']:
            if type(ele) == list:
                allNoneList = all(subEle == 'none' for subEle in ele)
            else:
                if ele != 'none':
                    allNoneList = False
            # At this point, a false result will exit the loop.
            if allNoneList == False:
                break

        # Determine whether all okurigana values are empty strings.  Assume true.
        allEmptyList = True
        for ele in row['okuriList']:
            if type(ele) == list:
                allEmptyList = all(subEle == '' for subEle in ele)
            else:
                if ele != '':
                    allEmptyList = False
            # At this point, a false result will exit the loop.
            if allEmptyList == False:
                break

        if allNoneList == False or allEmptyList == False:
            # Iterate on each element of affixList / okuriList for each kanji.
            sndx = 0
            for ndx in range(len(row['UCSList'])):
                print(f"const affix_enum kun_affix{kanaBytes}_{sndx:02d}[] = ", end='', file=f)
                ele = row['affixList'][ndx]
                if type(ele) == list:
                    arrSize = len(ele)
                    # Create a array of two or more affix enumeration values for a given kanji.
                    delimit = '{'
                    for affix in ele:
                        print(f"{delimit}{affix}", end='', file=f)
                        delimit = ', '

                    print('};', file=f)
                else:
                    arrSize = 1
                    # Create array containing a single affix enumeration for a given kanji.
                    print(f"{open_curl}{ele}{close_curl};", file=f)

                print(f"const char *kun_olist{kanaBytes}_{sndx:02d}[] = ", end='', file=f)
                ele = row['okuriList'][ndx]
                if type(ele) == list:
                    # Create array of two or more okurigana strings for a given kanji.
                    delimit = '{'
                    for okuri in ele:
                        print(f"{delimit}\"", end='', file=f)
                        # Create array of UTF-8 char strings for the okurigana.
                        odd = True
                        for nybble in [*okuri.encode('utf8').hex()]:
                            if odd:
                                print(f"\\x{nybble}", end='', file=f)
                            else:
                                print(f"{nybble}", end='', file=f)

                            odd ^= True

                        print("\"", end='', file=f)
                        delimit = ', '

                    print('};', file=f)
                else:
                    # Create a UTF-8 char strings for the okurigana.
                    print("{\"", end='', file=f)
                    odd = True
                    for nybble in [*ele.encode('utf8').hex()]:
                        if odd:
                            print(f"\\x{nybble}", end='', file=f)
                        else:
                            print(f"{nybble}", end='', file=f)

                        odd ^= True

                    print("\"};", file=f)

                print(f"const okuri_md kun_okuri_md{kanaBytes}_{sndx:02d} = ", end='', file=f)
                print(f"{open_curl}{arrSize}, kun_affix{kanaBytes}_{sndx:02d}, kun_olist{kanaBytes}_{sndx:02d}{close_curl};", file=f)
                sndx += 1
            
            # Now generate array of okurigana structures of same length as kanji.
            print(f"const okuri_md * const kun_okuri{kanaBytes}[] = ", end='', file=f)
            delimit = '{'
            for ndx in range(sndx):
                print(f"{delimit}&kun_okuri_md{kanaBytes}_{ndx:02d}", end='', file=f)
                delimit = ", "

            print("};", file=f)

        # Generate the Kanji data structure for this reading.
        delimit = '{'
        print(f"const kanji_md * const kun_kanji{kanaBytes}[] = ", end='', file=f)
        for ucs in row['UCSList']:
            ucs_name = str.upper(ucs)
            print(f"{delimit}&kanji_md{ucs_name}", end='', file=f)
            delimit = ', '

        print('};', file=f)
        print(f"const reading_md kunyomi_md{kanaBytes} = ", end='', file=f)

        print(f"{open_curl}{len(row['UCSList'])},", end='', file=f)
        if allNoneList and allEmptyList:
            print(f" NULL,", end='', file=f)
        else:
            print(f" kun_okuri{kanaBytes},", end='', file=f)
    
        print(f" kun_kanji{kanaBytes}{close_curl};", file=f)

    # Data type 3: Node for balanced binary tree.
    # typedef struct bbt_node {
    #   const uint32_t key;
    #   const reading_md* rmd;
    #   const bbt_node * const lnode;
    #   const bbt_node * const rnode;
    # };
    print("\n#ifndef BBT_NODE_TYPE", file=f)
    print("#define BBT_NODE_TYPE", file=f)
    print("typedef struct bbt_node {", file=f)
    print("  const uint32_t key;", file=f)
    print("  const reading_md* rmd;", file=f)
    print("  const bbt_node * const lnode;", file=f)
    print("  const bbt_node * const rnode;", file=f)
    print("} bbt_node;", file=f)
    print("#endif", file=f)

    # Pull out the KanaBytes and KanaInt colums from the sorted
    # onyomi DataFrame (df2Sort).
    df3Sub = df3Sort[['KanaBytes', 'KanaInt']]
    # Sort according to KanaInt.
    df3SubSort = df3Sub.sort_values('KanaInt', ignore_index=True)
    # Form a list from the sorted KanaBytes column.
    kunyomiList = df3SubSort['KanaBytes'].tolist()
    # Generate the node data structures for the Balanced Binary Structure Tree.
    root_node = print_balanced_bst(kunyomiList, "kunyomi", f)
    # Generate a pointer to the root node.
    print(f"const bbt_node *kunyomi_root_node = &kunyomi_node{root_node};", file=f)

f.close()
# END OF KUNYOMI OUTPUT PROCESSING

# START OF NANORI OUTPUT PROCESSING
# Add column to hold integer value that is the decimal representation of the
# concatenated lower bytes of the Katakana characters of the Onyomi string.
# After all rows are processed, none should contain the default value.
df4Sort['kanaBytes'] = ""
df4Sort['KanaInt'] = 0

# Enumeration for describing enhancements to onyomi, kunyomi, and
# nanori readings, and jukujikun.
# enum affix_enum {
#   none,   // No enhancments to reading (e.g., xyz).
#   prefix, // Reading is a prefix (e.g., wxyz-).
#   suffix, // Reading is a suffix (e.g., -wxyz).
#   jword,  // Japanese word string with jukujikun or irregular reading.
#   meaning // English meaning string for Japanese word enumerated above.
# };

# Data Type 2: Structure typedef of metadata for Okurigana (and other readings)
# typedef struct okuri_md {
#   const uint8_t len;      // Number of enumerations/character arrays.
#   const affix_enum *alist; // Pointer ot array of enumerations, one for each character array.
#   const char **clist;      // Pointer to array of character arrays, for for each enumeration.
# } okuri_md;

# Data Type 3: Structure typedef of metadata for Reading
# typedef struct reading_md {
#   const uint16_t len;             // Number of Okurigana and Kanji structures.
#   const okuri_md * const * olist; // Pointer to array of okurigana metadata structures.
#   const kanji_md * const * klist; // Pointer to array of kanji metadata structures.
# } reading_md;
# Data structure 3:  Array of Kanji metadata structures referenced by a
# specific onyomi, kunyomi, or nanori.  The order of the Kanji metadata
# structures in the list is the ascending order of Kanji frequency rank
# The value of XXXX is the numeric represenation of the kana for the
# onyomi, kunyomi, or nanori.
# const kanji_md const kmXXXX[] = {&kanji_md<UCS1>, &kanji_md<UCS2>, ... &kanji_md<UCSn>};
open_curl = "{"
close_curl = "}"
with open("nanori.h", "w") as f:
    print("/* nanori.h */\n", file=f)
    print("#ifndef AFFIX_ENUM", file=f)
    print("#define AFFIX_ENUM", file=f)
    print("enum affix_enum {", file=f)
    print("  none,   // No enhancments to reading (e.g., xyz).", file=f)
    print("  prefix, // Reading is a prefix (e.g., wxyz-).", file=f)
    print("  suffix, // Reading is a suffix (e.g., -wxyz).", file=f)
    print("  jword,  // Japanese word string with jukujikun or irregular reading.", file=f)
    print("  meaning // English meaning string for Japanese word enumerated above.", file=f)
    print("};", file=f)
    print("#endif\n", file=f)

    print("#ifndef OKURI_METADATA_TYPE", file=f)
    print("#define OKURI_METADATA_TYPE", file=f)
    print("typedef struct okuri_md {", file=f)
    print("  const uint8_t len;", file=f)
    print("  const affix_enum * const alist;", file=f)
    print("  const char **clist;", file=f)
    print("} okuri_md;", file=f)
    print("#endif\n", file=f)

    print("#ifndef READING_METADATA_TYPE", file=f)
    print("#define READING_METADATA_TYPE", file=f)
    print("typedef struct reading_md {", file=f)
    print("  const uint16_t len;", file=f)
    print("  const okuri_md * const * olist;", file=f)
    print("  const kanji_md * const * klist;", file=f)
    print("} reading_md;", file=f)
    print("#endif", file=f)

    # Iterate on each onyomi to create an array of kanji metadata for each nanori.
    for df4_ndx, row in df4Sort.iterrows():
        # Convert syllables of Kana into a number string for naming and
        # referencing of structure array.
        kanaBytes = ""
        for kana in row['Nanori']:
            kanaBytes = kanaBytes + kanaMap[kana]

        # Convert to a byte string.
        kanaBytesStr = bytes.fromhex(kanaBytes)
        # Generate 32-bit hash
        kanaInt = mmh3.hash(kanaBytesStr, signed=False)
        kanaBytes = hex(kanaInt)
        # Fill KanaBytes and KanaInt columns for the row.
        df4Sort.at[df4_ndx, 'KanaBytes'] = kanaBytes
        df4Sort.at[df4_ndx, 'KanaInt'] = kanaInt

        # There are no affix values.
        # Determine whether all okurigana values are empty strings.  Assume true.
        allEmptyList = True
        for ele in row['okuriList']:
            if type(ele) == list:
                allEmptyList = all(subEle == '' for subEle in ele)
            else:
                if ele != '':
                    allEmptyList = False
            # At this point, a false result will exit the loop.
            if allEmptyList == False:
                break

        if allEmptyList == False:
            # Iterate on each element of affixList / okuriList for each kanji.
            sndx = 0
            for ndx in range(len(row['UCSList'])):
                print(f"const char *na_olist{kanaBytes}_{sndx:02d}[] = ", end='', file=f)
                ele = row['okuriList'][ndx]
                if type(ele) == list:
                    # Create array of two or more okurigana strings for a given kanji.
                    delimit = '{'
                    for okuri in ele:
                        print(f"{delimit}\"", end='', file=f)
                        # Create array of UTF-8 char strings for the okurigana.
                        odd = True
                        for nybble in [*okuri.encode('utf8').hex()]:
                            if odd:
                                print(f"\\x{nybble}", end='', file=f)
                            else:
                                print(f"{nybble}", end='', file=f)

                            odd ^= True

                        print("\"", end='', file=f)
                        delimit = ', '

                    print('};', file=f)
                else:
                    # Create a UTF-8 char strings for the okurigana.
                    print("{\"", end='', file=f)
                    odd = True
                    for nybble in [*ele.encode('utf8').hex()]:
                        if odd:
                            print(f"\\x{nybble}", end='', file=f)
                        else:
                            print(f"{nybble}", end='', file=f)

                        odd ^= True

                    print("\"};", file=f)

                print(f"const okuri_md na_okuri_md{kanaBytes}_{sndx:02d} = ", end='', file=f)
                print(f"{open_curl}{arrSize}, NULL, na_olist{kanaBytes}_{sndx:02d}{close_curl};", file=f)
                sndx += 1
            
            # Now generate array of okurigana structures of same length as kanji
            print(f"const okuri_md * const na_okuri{kanaBytes}[] = ", end='', file=f)
            delimit = '{'
            for ndx in range(sndx):
                print(f"{delimit}&na_okuri_md{kanaBytes}_{ndx:02d}", end='', file=f)
                delimit = ", "

            print("};", file=f)

        delimit = '{'
        print(f"const kanji_md * const na_kanji{kanaBytes}[] = ", end='', file=f)
        for ucs in row['UCSList']:
            ucs_name = str.upper(ucs)
            print(f"{delimit}&kanji_md{ucs_name}", end='', file=f)
            delimit = ', '

        print('};', file=f)
        print(f"const reading_md nanori_md{kanaBytes} = ", end='', file=f)
        if allEmptyList:
            print(f"{open_curl}{len(row['UCSList'])}, NULL, na_kanji{kanaBytes}{close_curl};", file=f)
        else:
            print(f"{open_curl}{len(row['UCSList'])}, na_okuri{kanaBytes}, na_kanji{kanaBytes}{close_curl};", file=f)
    
    # Data type 3: Node for balanced binary tree.
    # typedef struct bbt_node {
    #   const uint32_t key;
    #   const onyomi_md* onmd;
    #   const bbt_node * const lnode;
    #   const bbt_node * const rnode;
    # };
    print("\n#ifndef BBT_NODE_TYPE", file=f)
    print("#define BBT_NODE_TYPE", file=f)
    print("typedef struct bbt_node {", file=f)
    print("  const uint32_t key;", file=f)
    print("  const reading_md* md;", file=f)
    print("  const bbt_node * const lnode;", file=f)
    print("  const bbt_node * const rnode;", file=f)
    print("} bbt_node;", file=f)
    print("#endif", file=f)

    # Pull out the KanaBytes and KanaInt colums from the sorted
    # onyomi DataFrame (df2Sort).
    df4Sub = df4Sort[['KanaBytes', 'KanaInt']]
    # Sort according to KanaInt.
    df4SubSort = df4Sub.sort_values('KanaInt', ignore_index=True)
    # Form a list from the sorted KanaBytes column.
    nanoriList = df4SubSort['KanaBytes'].tolist()
    # Generate the node data structures for the Balanced Binary Structure Tree.
    root_node = print_balanced_bst(nanoriList, "nanori", f)
    # Generate a pointer to the root node.
    print(f"const bbt_node *nanori_root_node = &nanori_node{root_node};", file=f)

f.close()

# END OF NANORI OUTPUT PROCESSING

print("Building the dictionary, starting with the Core 10K List.")

# Load in the Core 10K CSV file.
dfC10K = pd.read_csv('./Core10k.csv', sep='\t')
print(f"There are {len(dfC10K.index)} readings in Core 10K.")

dfPart1 = dfC10K[['Reading', 'Kanji', 'Definition']].copy()
# Add the rank column with a default value of -1.
# Add the source column with a default value of 'Core10K'.
dfPart1['Rank'] = -1
dfPart1['Source'] = 'Core10K'

print(f"The part dictionary starts with {len(dfPart1.index)} readings.")

# Load in the modified Core 5K Frequency (Nayr) CSV file.
dfC5K = pd.read_csv('./Core5kFrequencyMod3.csv', sep='\t')
print(f"There are {len(dfC5K.index)} readings in Core 5K.")

# Iterate on each row.
for dfC5K_ndx, row in dfC5K.iterrows():
    # Determine if each Kanji in the Word is one of the 6100+ most common Kanji.
    goodWord = True
    for kanji in row['Kanji']:
        # Skip non-kanji characters.
        if kanji < '一':
            continue
        if len(df1Subset.loc[df1Subset['Kanji'] == kanji]) == 0:
            print('Kanji not found')
            goodWord = False

for c5k_ndx, row in dfC5K.iterrows():
    # Find any matching readings in the full set for the C5K reading.
    rndxList = dfC10K.index[dfC10K['Reading'] == row['Reading']].tolist()
    if len(rndxList) > 0:
        # For matched readings, iterate on each one to search for Kanji.
        unmatched = True
        for rndx in rndxList:
            # Check if the C5K Kanji entry matches any of them.
            if dfC10K.iloc[rndx]['Kanji'] == row['Kanji']:
                # Core 5K is ordered according to Frequency (highest first), so
                # use index of row in Core 5K dataframe as rank (more or less).
                # dfPart1 is a copy of Core 10K for any row matched.
                dfPart1.at[rndx, 'Rank'] = row['#']
                unmatched = False
                #break
    else:
        unmatched = True
        
    if unmatched:
        # Core 5K is ordered according to Frequency (highest first), so
        # use index of row in Core 5K dataframe as rank (more or less).
        new_row = pd.DataFrame([[row['Reading'], row['Kanji'], row['Definition'], row['#'], 'Core5K']],
                                    columns=['Reading', 'Kanji', 'Definition', 'Rank', 'Source'])
        dfPart1 = pd.concat([dfPart1, new_row], ignore_index=True)

print(f"The part dictionary now has {len(dfPart1.index)} readings.")
dfPart2 = dfPart1.copy()

# Load in the modified Core 6K CSV file.
dfC6K = pd.read_csv('./Core6kMod.csv', sep='\t')

for dfC6K_ndx, row in dfC6K.iterrows():
    # Determine if each Kanji in the Word is one of the 6100+ most common Kanji.
    goodWord = True
    for kanji in row['Kanji']:
        # Skip non-kanji characters
        if kanji < '一':
            continue
        if len(df1Subset.loc[df1Subset['Kanji'] == kanji]) == 0:
            print('Kanji not found')
            goodWord = False

print(f"There are {len(dfC6K.index)} readings in Core 6K.")
for c6k_ndx, row in dfC6K.iterrows():
    # Find any matching readings in the full set for the C6K reading.
    rndxList = dfPart1.index[dfPart1['Reading'] == row['Reading']].tolist()
    if len(rndxList) > 0:
        # For matched readings, iterate on each one to search for Kanji.
        unmatched = True
        for rndx in rndxList:
            # Check if the C6K Kanji entry matches any of them.
            if dfPart1.iloc[rndx]['Kanji'] == row['Kanji']:
                unmatched = False
                break
    else:
        unmatched = True
        
    if unmatched:
        new_row = pd.DataFrame([[row['Reading'], row['Kanji'], row['Definition'], -1, 'Core6K']],
                                    columns=['Reading', 'Kanji', 'Definition', 'Rank', 'Source'])
        dfPart2 = pd.concat([dfPart2, new_row], ignore_index=True)

print(f"The part dictionary now has {len(dfPart2.index)} readings.")
dfPart3 = dfPart2.copy()

# Load in the modified Jukujikun readings file.
dfJuku = pd.read_csv('./jukujikun_mod.txt', sep='\t')

for dfJuku_ndx, row in dfJuku.iterrows():
    # Determine if each Kanji in the Word is one of the 6100+ most common Kanji.
    goodWord = True
    for kanji in row['Word']:
        # Skip non-kanji characters
        if kanji < '一':
            continue
        if len(df1Subset.loc[df1Subset['Kanji'] == kanji]) == 0:
            print('Kanji not found')
            goodWord = False

print(f"There are {len(dfJuku.index)} formal and informal Jukujikun readings")
# Iterate on the rows of the Jukujikun readings
for juku_ndx, row in dfJuku.iterrows():
    # For each Jukjikun reading entry, search for
    # Find any matching readings in the full set for the Jukujikun reading.
    rndxList = dfPart2.index[dfPart2['Reading'] == row['Reading']].tolist()
    if len(rndxList) > 0:
        # For matched readings, iterate on each one to search for Kanji.
        unmatched = True
        for rndx in rndxList:
            # Check if the C6K Kanji entry matches any of them.
            if dfPart2.iloc[rndx]['Kanji'] == row['Word']:
                unmatched = False
                break
    else:
        unmatched = True
        
    if unmatched:
        new_row = pd.DataFrame([[row['Reading'], row['Word'], row['Meaning'], -1, 'Juku']],
                                    columns=['Reading', 'Kanji', 'Definition', 'Rank', 'Source'])
        dfPart3 = pd.concat([dfPart3, new_row], ignore_index=True)

print(f"The part dictionary now has {len(dfPart3.index)} readings.")

# Load the 44492 Japanese words frequency file
df44492 = pd.read_csv('./44492-japanese-words-latin-lines-removed.txt', sep='\t')

for ndxLemma, row in df44492.iterrows():
    rndxList = dfPart3.index[dfPart3['Kanji'] == row['Lemma']].tolist()
    if len(rndxList) == 1:
        # Note that if rank has already been set to a positive number,
        # it will be overwritten.
        dfPart3.at[rndxList[0], 'Rank'] = ndxLemma + 1
    elif len(rndxList) > 1:
        #print(f"There are {len(rndxList)} rows in the Full dictionary with the same Kanji: {rndxList}")
        for ndx in rndxList:
            dfPart3.at[ndx, 'Rank'] = ndxLemma + 1

# Determine which words did not get ranked.
noRankList = dfPart3.index[dfPart3['Rank'] == -1].tolist()
print(f"After applying ranking, there are {len(noRankList)} entries without ranking.")

# Remove unranked words from the dictionary.
dfDict1 = dfPart3.drop(noRankList)
print(f"The initial dictionary has {len(dfDict1)} entries.")

print("Removing dictionary entries where there only Kana in the 'Kanji'.")
# Remove dictionary rows with 'Kanji' entries containing no Kanji.
noKanjiList = []
for dfDict1_ndx, row in dfDict1.iterrows():
    # Iterate on each character in 'Kanji', removing any that are actually all Kana.
    dropWord = True
    for kanjiChar in row['Kanji']:
        if kanjiChar >= '一':
            dropWord = False
            break

    # If word has some Kanji, add it to the new list.
    if dropWord == True:
        noKanjiList.append(dfDict1_ndx)        
        
dfDict2 = dfDict1.drop(noKanjiList).sort_values('Rank', ignore_index=True)
print(f"There are {len(dfDict2)} entries with Kanji")

# Now that the dictionary is ordered according to rank, drop a desired number of
# word with rank greater than a specified value.
# maxRank = 10000 # There are 5106 readings (4667 unique) for 5088 words.
# maxRank = 20000 # There are 6970 readings (6281 unique) for 6948 words. TOO BIG!
# maxRank = 15000 # There are 6208 readings (5623 unique) for 6187 words. STILL ROOM.
# maxRank = 17500 # There are 6619 readings (5976 unique) for 6597 words. TOO BIG!
# maxRank = 16000 # There are 6404 readings (5794 unique) for 6382 words. TOO BIG!
# maxRank = 15500 # There are 6298 readings (5701 unique) for 6277 words. TOO BIG!
# maxRank = 15400 # There are 6280 readings (5685 unique) for 6259 words. TOO BIG!
# maxRank = 15300 # There are 6264 readings (5672 unique) for 6243 words. TOO BIG!
# maxRank = 15250 # There are 6256 readings (5665 unique) for 6235 words. STILL ROOM.
# maxRank = 15275 # There are 6260 readings (5669 unique) for 6239 words. STILL ROOM.
# maxRank = 15290 # There are 6261 readings (5670 unique) for 6240 words. STILL ROOM.
# maxRank = 15295 # There are 6263 readings (5671 unique) for 6242 words. TOO BIG!
# maxRank = 15299 # There are 6264 readings (5672 unique) for 6243 words. TOO BIG!
# maxRank = 15298 # There are 6263 readings (5671 unique) for 6242 words. TOO BIG!
# maxRank = 15294 # There are 6262 readings (5671 unique) for 6241 words. TOO BIG!
# maxRank = 15293 # There are 6262 readings (5671 unique) for 6241 words. TOO BIG!
#maxRank = 15292 # There are 6260 readings (5670 unique) for 6239 words. OK!
#maxRank = 15295 # There are 6262 readings (5671 unique) for 6241 words. TOO BIG!
maxRank = 15294 # There are 6261 readings (5671 unique) for 6240 words. OK!
print(f"Dropping words with rank greater than {maxRank}.")
dropList = dfDict2.index[dfDict2['Rank'] > maxRank].tolist()
dfDict3 = dfDict2.drop(dropList)
print(f"There are now {len(dfDict3)} readings for words containing Kanji.")
print("Note that there may be multiple entries with the same reading and even")
print("cases with the same reading and Kanji, that have different meanings.")
# Create a new, empty data frame to hold the restructured data.
# The idea is to have a reading with one or more Kanji.
df5 = pd.DataFrame(columns = ['Reading', 'WordList', 'MeaningList'])

wordcount = 0
for row_ndx, row in dfDict3.iterrows():
    # Attempt to find the dictionary reading in DataFrame 5.
    ndxList = df5.index[df5['Reading'] == row['Reading']].tolist()
    if len(ndxList) != 0:
        # The rewading is present and there should be only one instance.
        ndx = ndxList[0]
        # Check to see whether the Kanji word is already present.
        if row['Kanji'] not in df5.iloc[ndx]['WordList']:
            # Add the Kanji value to the word list.
            df5.iloc[ndx]['WordList'].append(row['Kanji'])
            # Add meaning value to list for the dictionary.
            df5.iloc[ndx]['MeaningList'].append(row['Definition'])
            wordcount += 1
        else:
            # Find index of WordList matching word.
            kndx = df5.iloc[ndx]['WordList'].index(row['Kanji'])
            # Append definition to existing one in MeaningList at same index.
            df5.at[ndx, 'MeaningList'][kndx] = df5.iloc[ndx]['MeaningList'][kndx] + ", " + row['Definition']
    else:
        # The disctionary reading is not present in DataFrame 6, so add it,
        # as well as the kanji and meaning values.
        new_row = pd.DataFrame([[row['Reading'], [row['Kanji']], [row['Definition']]]],
                                columns=['Reading', 'WordList', 'MeaningList'])
        # Add the new row into DataFrame 6.
        df5 = pd.concat([df5, new_row], ignore_index=True)
        wordcount += 1

print(f"There are {len(df5)} unique readings for {wordcount} words.")
df6Sort = df5.sort_values('Reading', ignore_index=True)

# START OF DICTIONARY OUTPUT PROCESSING
# Add column to hold integer value that is the decimal representation of the
# concatenated lower bytes of the Katakana characters of the Word string.
# After all rows are processed, none should contain the default value.
df6Sort['kanaBytes'] = ""
df6Sort['KanaInt'] = 0

# Enumeration for describing enhancements to onyomi, kunyomi, and
# nanori readings, and dictionary words.
# enum affix_enum {
#   none,   // No enhancments to reading (e.g., xyz).
#   prefix, // Reading is a prefix (e.g., wxyz-).
#   suffix, // Reading is a suffix (e.g., -wxyz).
#   jword,  // Japanese word string with jukujikun or irregular reading.
#   meaning // English meaning string for Japanese word enumerated above.
# };

# Data Type 2: Structure typedef of metadata for Okurigana (and other readings)
# typedef struct okuri_md {
#   const uint8_t len;      // Number of enumerations/character arrays.
#   const affix_enum *alist; // Pointer ot array of enumerations, one for each character array.
#   const char **clist;      // Pointer to array of character arrays, for for each enumeration.
# } okuri_md;

# Data Type 3: Structure typedef of metadata for Reading
# typedef struct reading_md {
#   const uint16_t len;             // Number of Okurigana and Kanji structures.
#   const okuri_md * const * olist; // Pointer to array of okurigana metadata structures.
#   const kanji_md * const * klist; // Pointer to array of kanji metadata structures.
# } reading_md;
# Data structure 3:  Array of Kanji metadata structures referenced by a
# specific onyomi, kunyomi, or nanori.  The order of the Kanji metadata
# structures in the list is the ascending order of Kanji frequency rank
# The value of XXXX is the numeric represenation of the kana for the
# onyomi, kunyomi, or nanori.
# const kanji_md const kmXXXX[] = {&kanji_md<UCS1>, &kanji_md<UCS2>, ... &kanji_md<UCSn>};

with open("dictionary.h", "w") as f:
    print("/* dictionary.h */\n", file=f)
    print("#ifndef AFFIX_ENUM", file=f)
    print("#define AFFIX_ENUM", file=f)
    print("enum affix_enum {", file=f)
    print("  none,   // No enhancments to reading (e.g., xyz).", file=f)
    print("  prefix, // Reading is a prefix (e.g., wxyz-).", file=f)
    print("  suffix, // Reading is a suffix (e.g., -wxyz).", file=f)
    print("  jword,  // Japanese word string with jukujikun or irregular reading.", file=f)
    print("  meaning // English meaning string for Japanese word enumerated above.", file=f)
    print("};", file=f)
    print("#endif\n", file=f)

    print("#ifndef OKURI_METADATA_TYPE", file=f)
    print("#define OKURI_METADATA_TYPE", file=f)
    print("typedef struct okuri_md {", file=f)
    print("  const uint8_t len;", file=f)
    print("  const affix_enum * const alist;", file=f)
    print("  const char **clist;", file=f)
    print("} okuri_md;", file=f)
    print("#endif\n", file=f)

    print("#ifndef READING_METADATA_TYPE", file=f)
    print("#define READING_METADATA_TYPE", file=f)
    print("typedef struct reading_md {", file=f)
    print("  const uint16_t len;", file=f)
    print("  const okuri_md * const * olist;", file=f)
    print("  const kanji_md * const * klist;", file=f)
    print("} reading_md;", file=f)
    print("#endif", file=f)

    # Iterate on each dictionary to create an array of kanji metadata for each dictionary value.
    for df6_ndx, row in df6Sort.iterrows():
        # Convert syllables of Kana into a number string for naming and
        # referencing of structure array.
        kanaBytes = ""
        for kana in row['Reading']:
            kanaBytes = kanaBytes + kanaMap[kana]

        # Convert to a byte string.
        kanaBytesStr = bytes.fromhex(kanaBytes)
        # Generate 32-bit hash
        kanaInt = mmh3.hash(kanaBytesStr, signed=False)
        # Make string of Kana bytes look like a hex number.
        kanaBytes = hex(kanaInt)
        # Fill KanaBytes and KanaInt columns for the row.
        df6Sort.at[df6_ndx, 'KanaBytes'] = kanaBytes
        df6Sort.at[df6_ndx, 'KanaInt'] = kanaInt

        # Iterate on each element of affixList / okuriList for each kanji.
        sndx = 0
        for ndx in range(len(row['WordList'])):
            # Generate the enumeration array.
            print(f"const affix_enum dict_affix{kanaBytes}_{sndx:02d}[] = ", end='', file=f)
            print(f"{open_curl}jword, meaning{close_curl};", file=f)
            # Generate the character string list.
            # Create a UTF-8 char strings for the okurigana.
            print(f"const char *dict_olist{kanaBytes}_{sndx:02d}[] = ", end='', file=f)
            jword = row['WordList'][ndx]
            # Create a UTF-8 char strings for the Japanese word.
            print(f"{open_curl}\"", end='', file=f)
            odd = True
            for nybble in [*jword.encode('utf8').hex()]:
                if odd:
                    print(f"\\x{nybble}", end='', file=f)
                else:
                    print(f"{nybble}", end='', file=f)

                odd ^= True

            print(f"\", \"{row['MeaningList'][ndx]}\"{close_curl};", file=f)
            print(f"const okuri_md dict_okuri_md{kanaBytes}_{sndx:02d} = ", end='', file=f)
            print(f"{open_curl}2, dict_affix{kanaBytes}_{sndx:02d}, ", end='', file=f)
            print(f"dict_olist{kanaBytes}_{sndx:02d}{close_curl};", file=f)
            sndx += 1

        # Now generate array of okurigana structures of same length as kanji.
        print(f"const okuri_md * const dict_okuri{kanaBytes}[] = ", end='', file=f)
        delimit = '{'
        for ndx in range(sndx):
            print(f"{delimit}&dict_okuri_md{kanaBytes}_{ndx:02d}", end='', file=f)
            delimit = ", "

        print("};", file=f)
        print(f"const reading_md dictionary_md{kanaBytes} = ", end='', file=f)
        print(f"{open_curl}{len(row['WordList'])},", end='', file=f)
        print(f" dict_okuri{kanaBytes}, NULL{close_curl};", file=f)

    # Data type 3: Node for balanced binary tree.
    # typedef struct bbt_node {
    #   const uint32_t key;
    #   const onyomi_md* onmd;
    #   const bbt_node * const lnode;
    #   const bbt_node * const rnode;
    # };
    print("\n#ifndef BBT_NODE_TYPE", file=f)
    print("#define BBT_NODE_TYPE", file=f)
    print("typedef struct bbt_node {", file=f)
    print("  const uint32_t key;", file=f)
    print("  const reading_md* md;", file=f)
    print("  const bbt_node * const lnode;", file=f)
    print("  const bbt_node * const rnode;", file=f)
    print("} bbt_node;", file=f)
    print("#endif", file=f)

    # Pull out the KanaBytes and KanaInt colums from the sorted
    # jukujikan DataFrame (dfJukuSort).
    df6Sub = df6Sort[['KanaBytes', 'KanaInt']]
    # Sort according to KanaInt.
    df6SubSort = df6Sub.sort_values('KanaInt', ignore_index=True)
    # Form a list from the sorted KanaBytes column.
    dictionaryList = df6SubSort['KanaBytes'].tolist()
    # Generate the node data structures for the Balanced Binary Structure Tree.
    root_node = print_balanced_bst(dictionaryList, "dictionary", f)
    print(f"const bbt_node *dictionary_root_node = &dictionary_node{root_node};", file=f)

f.close()
# END OF DICTIONARY OUTPUT PROCESSING

print("Done")
