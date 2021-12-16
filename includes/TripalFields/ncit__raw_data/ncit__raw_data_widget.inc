<?php
/**
 * @class
 * Purpose:
 *
 * Allowing edit?
 * Data:
 * Assumptions:
 */
class ncit__raw_data_widget extends TripalFieldWidget {

  // The default label for this field.
  public static $default_label = 'No Edits';

  // The list of field types for which this formatter is appropriate.
  public static $field_types = array('ncit__raw_data');

 
 /**
  * @see TripalFieldWidget::form()
  *
  **/

  public function form(&$widget, &$form, &$form_state, $langcode, $items, $delta, $element) {
    parent::form($widget, $form, $form_state, $langcode, $items, $delta, $element);
  }

  
 /**
  * @see TripalFieldWidget::validate()
  *
  **/
  public function validate($element, $form, &$form_state, $langcode, $delta) {
  }

  
 /**
  * @see TripalFieldWidget::submit()
  *
  **/
  public function submit($form, &$form_state, $entity_type, $entity, $langcode, $delta) {
  }

}