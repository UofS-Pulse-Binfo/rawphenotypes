<?php
/**
 * @file 
 * Construct configuration form to manage Rawphenotypes module settings.
 */

namespace Drupal\rawphenotypes\Form;

use Drupal\Core\Form\ConfigFormBase;
use Drupal\Core\Form\FormStateInterface;

/**
 * Defines RawphenotypesConfigurationForm class.
 */
class RawphenotypesConfigurationForm extends ConfigFormBase {
  const SETTINGS = 'rawphenotypes.settings';
  
  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'rawphenotypes_settings';
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

    // Colour scheme.
    $form['fieldset_colour_scheme'] = [
      '#type' => 'details',
      '#title' => $this->t('Colour Scheme'),
      '#open' => TRUE
    ];

      $form['fieldset_colour_scheme']['rawpheno_colour_scheme'] = [
        '#type' => 'textfield',
        '#title' => $this->t('Enter colour:'),
        '#default_value' => $config->get('rawpheno_colour_scheme'),
        '#size' => 40,
        '#maxlength' => 20,
        '#required' => TRUE,
        '#description' => $this->t('eg. HEX: #304356, blue'),
      ];
    
    // Headers and title.
    $form['fieldset_page_title'] = [
      '#type' => 'details',
      '#title' => $this->t('Page title'),
      '#open' => TRUE,
    ];
      // Rawdata page.
      $form['fieldset_page_title']['rawpheno_rawdata_title'] = [
        '#type' => 'textfield',
        '#title' => $this->t('Title of Rawdata page:'),
        '#default_value' => $config->get('rawpheno_rawdata_title'),
        '#size' => 120,
        '#maxlength' => 220,
        '#required' => TRUE,
      ];

      // Download Page.
      $form['fieldset_page_title']['rawpheno_download_title'] = [
        '#type' => 'textfield',
        '#title' => $this->t('Title of Download page:'),
        '#default_value' => $config->get('rawpheno_download_title'),
        '#size' => 120,
        '#maxlength' => 220,
        '#required' => TRUE,
      ];

      // Instructions page.
      $form['fieldset_page_title']['rawpheno_instructions_title'] = [
        '#type' => 'textfield',
        '#title' => $this->t('Title of Instructions page:'),
        '#default_value' => $config->get('rawpheno_instructions_title'),
        '#size' => 120,
        '#maxlength' => 220,
        '#required' => TRUE,
      ];

      // Upload page.
      $form['fieldset_page_title']['rawpheno_upload_title'] = [
        '#type' => 'textfield',
        '#title' => $this->t('Title of Upload page:'),
        '#default_value' => $config->get('rawpheno_upload_title'),
        '#size' => 120,
        '#maxlength' => 220,
        '#required' => TRUE,
      ];

      // Backup page.
      $form['fieldset_page_title']['rawpheno_backup_title'] = [
        '#type' => 'textfield',
        '#title' => $this->t('Title of Backup page:'),
        '#default_value' => $config->get('rawpheno_backup_title'),
        '#size' => 120,
        '#maxlength' => 220,
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
      ->set('rawpheno_colour_scheme', $form_state->getValue('rawpheno_colour_scheme'))
      ->set('rawpheno_rawdata_title', $form_state->getValue('rawpheno_rawdata_title'))
      ->set('rawpheno_download_title', $form_state->getValue('rawpheno_download_title'))
      ->set('rawpheno_instructions_title', $form_state->getValue('rawpheno_instructions_title'))
      ->set('rawpheno_upload_title', $form_state->getValue('rawpheno_upload_title'))
      ->set('rawpheno_backup_title', $form_state->getValue('rawpheno_backup_title'))
      ->set('rawpheno_rtransform_words', $form_state->getValue('rawpheno_rtransform_words'))
      ->set('rawpheno_rtransform_characters', $form_state->getValue('rawpheno_rtransform_characters'))
      ->set('rawpheno_rtransform_replace', $form_state->getValue('rawpheno_rtransform_replace'))
      ->save();

    return parent::submitForm($form, $form_state);
  }
}