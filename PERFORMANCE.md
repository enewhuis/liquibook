Performance Test Results, Inserts Per Second
============================================
(newest results on top)
 
<table>
  <tr>
    <th>5 Level Depth</th>
    <th>BBO Only</th>
    <th>Order Book Only</th>
    <th>Note</th>
  </tr>
  <tr>
    <td>2,062,158</td>
    <td>2,139,950</td>
    <td>2,494,532</td>
    <td>Now testing on a modern laptop (2.4 GHZ i7).</td>
  </tr>
  <tr>
    <td>1,231,959</td>
    <td>1,273,510</td>
    <td>1,506,066</td>
    <td>Handling all or none order condition.</td>
  </tr>
  <tr>
    <td>1,249,544</td>
    <td>1,305,482</td>
    <td>1,531,998</td>
    <td>Remove callbacks_added method.  Caller can invoke equivalent if necessary.</td>
  </tr>
  <tr>
    <td>1,222,000</td>
    <td>1,279,711</td>
    <td>1,495,714</td>
    <td>Use vector for callback container.</td>
  </tr>
  <tr>
    <td>1,250,616</td>
    <td>1,264,227</td>
    <td>1,463,738</td>
    <td>Union in callback.  For clarity of purpose, not for performance.</td>
  </tr>
  <tr>
    <td>1,267,135</td>
    <td>1,270,188</td>
    <td>1,469,246</td>
    <td>Combine 2 fill callbacks into one.</td>
  </tr>
  <tr>
    <td>1,233,894</td>
    <td>1,237,154</td>
    <td>1,434,354</td>
    <td>Store excess depth levels in depth to speed repopulation.</td>
  </tr>
  <tr>
    <td>58,936</td>
    <td>153,839</td>
    <td>1,500,874</td>
    <td>Removed spuroious insert on accept of completely filled order.</td>
  </tr>
  <tr>
    <td>38,878</td>
    <td>124,756</td>
    <td>1,495,744</td>
    <td>Initial run with all 3 tests.</td>
  </tr>
</table>

