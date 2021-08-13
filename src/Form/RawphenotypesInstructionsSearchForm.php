<?php
/**
 * @file 
 * Provide form api to instructions page.
 * Fields:
 * - Autocomplete search trait.
 */

namespace Drupal\rawphenotypes\Form;

use Drupal\Core\Form\FormBase;
use Drupal\Core\Form\FormStateInterface;

/**
 * Defines RawphenotypesInstructionSearchForm class.
 */
class RawphenotypesInstructionsSearchForm extends FormBase {
  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'rawphenotypes_search_trait_form';
  }

  /**
   * {@inheritdoc}
   * Build form.
   */
  public function buildForm(array $form, FormStateInterface $form_state) {    
    $form['txt_search'] = [
      '#title' => '',
      '#type' => 'textfield',
      '#maxlength' => 65,
      '#size' => 65,
      '#default_value' => $this->t('Search Trait'),
      '#theme_wrappers' => []
    ];

    $form['sel_project'] = array(
      '#type' => 'select',
      '#options' => array(0 => 'Please select a project'),
      '#id' => 'rawpheno-ins-sel-project'
    );

    return $form;
  }

  /**
   * {@inheritdoc}
   * Save configuration.
   */
  public function validateForm(array &$form, FormStateInterface $form_state) {
  }

  /**
   * {@inheritdoc}
   * Save configuration.
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {    
  }
}