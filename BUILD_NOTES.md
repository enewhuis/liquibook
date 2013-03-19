Build Notes
===========

<table>
  <tr>
    <th>Platform</th>
    <th>ACE or OpenDDS</th>
    <th>Configuration</th>
    <th>Command</th>
    <th>Note</th>
  </tr>
  <tr>
    <td>Linux</td>
    <td>No</td>
    <td>Release</td>
    <td>mwc.pl -type make liquibook.mwc</td>
    <td></td>
  </tr>
  <tr>
    <td>Linux</td>
    <td>No</td>
    <td>Debug</td>
    <td>mwc.pl -type make liquibook.mwc -value_template configurations=Debug</td>
    <td></td>
  </tr>
  <tr>
    <td>Linux</td>
    <td>Yes</td>
    <td>Both</td>
    <td>mwc.pl -type gnuace liquibook.mwc</td>
    <td>See note on platform_macros.GNU</td>
  </tr>
  <tr>
    <td>Windows</td>
    <td>Either</td>
    <td>Both</td>
    <td>mwc.pl -type vc10 liquibook.mwc</td>
    <td></td>
  </tr>
</table>

## GCC With ACE/OpenDDS: Building Release and Debug
To switch between __Debug__ and __Release__ builds, modify the file $ACE_ROOT/include/makeinclude/platform_macros.GNU:

Debug:
<pre>
boost=1
debug=1
inline=0
optimize=0
include $(ACE_ROOT)/include/makeinclude/platform_linux.GNU
</pre>

Release:
<pre>
boost=1
debug=0
inline=1
optimize=1
include $(ACE_ROOT)/include/makeinclude/platform_linux.GNU
</pre>


