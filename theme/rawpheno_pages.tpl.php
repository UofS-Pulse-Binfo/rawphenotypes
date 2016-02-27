<?php
 $page = $form['form_id']['#value'];
?>

<div class="container-header">
  <?php print drupal_render($form['page_title']); ?>
</div>

<div class="container-page">
  <div class="content-wrapper">
    <div class="container-subtitle">
      <div class="subtitle-left">
        <?php
          if ($page == 'rawpheno_instructions') {
            print drupal_render($form["txt_search"]); 
            print drupal_render($form["btn_search"]); 
            print drupal_render($form["json_url"]);
          }
        ?>
      </div>
      
      <div class="subtitle-right">
        <?php print drupal_render($form['page_button']); ?>
      </div>
    </div>
    
    <div class="container-contents">
      <?php 
        if ($page == 'rawpheno_rawdata') {
          print drupal_render($form['page_content']); 
          print drupal_render($form['json_url']); 
        }
        elseif ($page == 'rawpheno_instructions') {
          include_once('rawpheno_instructions.tpl.php');
        }
      ?>
    </div>
  </div>
</div>