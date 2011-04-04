# pietro gagliardi - 3-4 apr 2011
# format of input: ROM|crc32|Working%|gitversion|notes
# first line of input is page title
# Working% should either be the string Untested or a base-10 number [0,100]
BEGIN {
	FS = "|"
	getline title
	print "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\""
	print "   \"http://www.w3.org/TR/html4/strict.dtd\">"
	print "<html>"
	print "<head>"
	printf "\t<title>Gens/GS II Compatibility List: %s</title>\n", title
	print "\t<meta http-equiv='Content-Type' content='text/html; charset=utf-8'>"
	print "\t<link href=\"rom-compat.css\" rel=\"stylesheet\" type=\"text/css\">"
	print "</head>"
	print "<body>"
	print "<h1>Gens/GS II Compatibility List</h1>"
	printf "<h2>ROM List: %s</h2>\n", title
	print "<table>"
	print "\t<tr class='header'>"
	print "\t\t<th>ROM</th>"
	print "\t\t<th>CRC32</th>"
	print "\t\t<th>Working %</th>"
	print "\t\t<th>Tested With<br>git Version</th>"
	print "\t\t<th>Notes</th>"
	print "\t</tr>"
}

{
	# ROM name.
	name = $1
	gsub(/&/, "&amp;", name)
	
	# CRC32.
	crc32 = $2
	if (crc32 == "")
		crc32 = "&nbsp;"
	
	# Gens/GS II git version.
	version = $4
	if (version == "")
		version = "&nbsp;"
	
	# ROM notes.
	notes = $5
	gsub(/&/, "\\&amp;", notes)
	gsub(/</, "\\&lt;", notes)
	gsub(/>/, "\\&gt;", notes)
	gsub(/"/, "\\&quot;", notes)
	gsub(/\\n/, "<br>", notes)
	if (notes == "")
		notes = "&nbsp;"
	
	# Print the table row.
	printf "\t<tr class=\"pcnt_%s\">\n", $3
	printf "\t\t<td class='name'>%s</td>\n", name
	printf "\t\t<td class='crc32'>%s</td>\n", crc32
	printf "\t\t<td class='pcnt_working'>%d%%</td>\n", $3
	printf "\t\t<td class='version'>%s</td>\n", version
	printf "\t\t<td class='notes'>%s</td>\n", notes
	print "\t</tr>"
}

END {
	print "</table>"
	print "</body>"
	print "</html>"
}
