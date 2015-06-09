<?php

/**
 * @file
 *
 * Template file for theme('media_23video_oauth').
 */

?>
<?php foreach ($before as $id => $item): ?>
  <?php print drupal_render($item); ?>
<?php endforeach; ?>
<table>
  <tr>
  <?php foreach ($form['#header'] as $id => $header): ?>
    <th><?php print $header; ?></th>
  <?php endforeach; ?>
  </tr>
  <?php foreach ($roles as $id => $row): ?>
    <tr>
      <?php foreach ($row as $id => $subrow): ?>
        <?php if (substr($id, 0, 1) != '#'): ?>
          <td><?php print drupal_render($subrow); ?></td>
        <?php endif; ?>
      <?php endforeach ?>
    </tr>
  <?php endforeach; ?>
</table>
<?php print drupal_render($form['submit']); ?>
<?php foreach ($after as $id => $item): ?>
  <?php print drupal_render($item); ?>
<?php endforeach; ?>
<?php foreach ($rest as $id => $item): ?>
  <?php print drupal_render($item); ?>
<?php endforeach ?>
