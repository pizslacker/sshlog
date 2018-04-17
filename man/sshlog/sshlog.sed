1s/^\(.*\)$/\.TH \1/
/NAME/s/^\(.*\)$/\.SH \1/
/SYNOPSIS/s/^\(.*\)$/\.SH \1/
/DESCRIPTION/s/^\(.*\)$/\.SH \1/
/\<EXAMPLE\>/s/^\(.*\)$/\.SH \1/
/AUTHOR/s/^\(.*\)$/\.SH \1/

s/sshlog/\\fIsshlog\\fR/g

/EXAMPLEMARK/,/EXAMPLEMARK/{
a\
.br
}
/EXAMPLEMARK/s///
/11111/s///
/22222/s///

/SEE ALSO/,/AUTHOR/{
/^[a-z]/s/^\(.*\)$/\.BR \1/
s/(\([1-9]\))/ "(\1), "/g
s/(C++)/ "(C++), "/g
s/(C++)$/ "(C++)"/
}
/AUTHOR/{
i\

a\
      \\fIK\\fRim\ \\fIO\\fRlsen   <kim@pizslacker.org>
}
