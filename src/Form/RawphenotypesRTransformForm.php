<?php
/**
 * @file 
 * Construct configuration form to manage Rawphenotypes R transformation rules.
 * Rules:
 * - Words to remove: list of words to remove such as of or til.
 * - Special characters to remove: list of special characters to remove such as : or ;.
 * - Match and replace: match a character and proved a replacement of it.
 */

namespace Drupal\rawphenotypes\Form;

use Drupal\Core\Form\ConfigFormBase;
use Drupal\Core\Form\FormStateInterface;

/**
 * Defines RawphenotypesRTransformForm class.
 */
class RawphenotypesRTransformForm extends ConfigFormBase {
  const SETTINGS = 'rawphenotypes.settings';
  
  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'rawphenotypes_rtransform';
  }

  /**
   * {@inheritdoc}
   */
  protected function getEditableConfigNames() {
    return [
      static::SETTINGS,
    ];
  }

  /**
   * {@inheritdoc}
   * Build form.
   */
  public function buildForm(array $form, FormStateInterface $form_state) {
    // Attach library.
    $form['#attached']['library'][] = 'rawphenotypes/style-admin';
 
    $config = $this->config(static::SETTINGS);

    // Fieldset to contain R-Friendly transformation rules form.
    $form['fieldset_rules'] = [
      '#type' => 'details',
      '#title' => $this->t('R-Friendly Transformation Rules'),
      '#open' => TRUE,
    ];

      $form['fieldset_rules']['rawpheno_rtransform_words'] = [
        '#type' => 'textarea',
        '#title' => $this->t('List of words to remove:'),
        '#default_value' => $config->get('rawpheno_rtransform_words'),
        '#description' => $this->t('Separate words with commas'),
        '#required' => TRUE,
      ];

      $form['fieldset_rules']['rawpheno_rtransform_characters'] = [
        '#type' => 'textarea',
        '#title' => $this->t('List of special characters to remove:'),
        '#default_value' => $config->get('rawpheno_rtransform_characters'),
        '#description' => $this->t('Separate characters with commas'),
        '#required' => TRUE,
      ];

      $form['fieldset_rules']['rawpheno_rtransform_replace'] = [
        '#type' => 'textarea',
        '#title' => $this->t('Match and replace (Match Character = Replacement):'),
        '#default_value' => $config->get('rawpheno_rtransform_replace'),
        '#description' => $this->t('Separate match and replace pairs with commas'),
        '#required' => TRUE,
      ];
    
    $form['req'] = [
      '#type' => 'inline_template',
      '#template' => '<span>&nbsp;* means field is required</span>'
    ];  

    return parent::buildForm($form, $form_state);
  }

  /**
   * {@inheritdoc}
   * Save configuration.
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    $this->configFactory->getEditable(static::SETTINGS)
      ->set('rawpheno_rtransform_words', $form_state->getValue('rawpheno_rtransform_words'))
      ->set('rawpheno_rtransform_characters', $form_state->getValue('rawpheno_rtransform_characters'))
      ->set('rawpheno_rtransform_replace', $form_state->getValue('rawpheno_rtransform_replace'))
      ->save();

    return parent::submitForm($form, $form_state);
  }
}