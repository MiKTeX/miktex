// TeXworksScript
// Title: System test
// Description: (For demo purposes) Runs selection as a shell command, and replaces it with the output
// Author: Jonathan Kew
// Version: 0.3
// Date: 2010-01-09
// Script-Type: standalone
// Context: TeXDocument

var cmd = TW.target.selection;
if (cmd != "") {
  var result = TW.system(cmd);
  if (result.status == 0) {
    TW.target.insertText(result.output);
  }
  else {
    TW.target.insertText("ERROR: " + result.message);
  }
}
