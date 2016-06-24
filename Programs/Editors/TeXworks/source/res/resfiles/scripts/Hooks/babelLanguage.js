// TeXworksScript
// Title: Babel language
// Description: Looks for a Babel line to set the spell-check language
// Author: Jonathan Kew & Stefan Löffler
// Version: 0.3.1
// Date: 2012-03-20
// Script-Type: hook
// Hook: LoadFile

var spellcheckModlineRE = new RegExp("% *!TEX +spellcheck *=", "i");
var babelRE = new RegExp("^[^%]*\\\\usepackage\\[(?:.+,)*([^,]+)\\]\\{babel\\}");

var spellingDict = new Array();

// extend or customize this list as needed
spellingDict.czech     = "cs_CZ";
spellingDict.german    = "de_DE";
spellingDict.germanb   = "de_DE";
spellingDict.ngerman   = "de_DE";
spellingDict.greek     = "el_GR";
spellingDict.english   = "en_US";
spellingDict.USenglish = "en_US";
spellingDict.american  = "en_US";
spellingDict.UKenglish = "en_GB";
spellingDict.british   = "en_GB";
spellingDict.spanish   = "es_ES";
spellingDict.french    = "fr_FR";
spellingDict.francais  = "fr_FR";
spellingDict.latin     = "la_LA";
spellingDict.latvian   = "lv_LV";
spellingDict.polish    = "pl_PL";
spellingDict.brazilian = "pt_BR";
spellingDict.brazil    = "pt_BR";
spellingDict.portuges  = "pt_PT";
spellingDict.portuguese= "pt_PT";
spellingDict.russian   = "ru_RU";
spellingDict.slovak    = "sk_SK";
spellingDict.slovene   = "sl_SL";
spellingDict.swedish   = "sv_SV";

// get list of available dictionairies to avoid overriding the default spell
// checking language by "None" in case we tried to set the language to something
// for which no dict exists.
var dicts = TW.getDictionaryList();

function startsWith(haystack, needle)
{
  if (needle.length > haystack.length)
    return false;
  return haystack.substr(0, needle.length) === needle
}

function trySetSpellcheckLanguage(lang)
{
  // See if we have any entry for this in our list
  if (spellingDict[lang] === undefined)
    return false;
  // map the babel code to an ISO language code
  lang = spellingDict[lang];
  // If we have a matching dict, set it
  if (dicts[lang] !== undefined) {
    TW.target.setSpellcheckLanguage(lang);
    TW.result = "Set spell-check language to " + lang;
    return true;
  }
  // Otherwise, see if we have a specialized dict (e.g., de_DE_frami when de_DE
  // is requested).
  for (d in dicts) {
    if (startsWith(d, lang)) {
      TW.target.setSpellcheckLanguage(d);
      TW.result = "Set spell-check language to " + d;
      return true;
    }
  }
  return false;
}

// get the text from the document window
var txt = TW.target.text;
var lines = txt.split('\n');

// look for a babel line...
for (i = 0; i < lines.length; ++i) {
  var line = lines[i];
  // If we have a "%!TeX spellcheck" modline, we don't override it (after all,
  // the user has probably put it there for a reason)
  if (spellcheckModlineRE.test(line)) 
    break;
  var matched = babelRE.exec(line);
  if (matched) {
    if (trySetSpellcheckLanguage(matched[1]))
      break;
  }
  // ...but give up at the end of the preamble
  if (line.match("\\\\begin\\{document\\}"))
    break;
  if (line.match("\\\\starttext")) // oops, seems to be ConTeXt!
    break;
}
undefined;
