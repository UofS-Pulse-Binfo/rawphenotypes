<?php
/**
 * @file 
 * Construct configuration form to manage project assets.
 */

namespace Drupal\rawphenotypes\Form;

use Drupal\Core\Url;
use Drupal\file\Entity\File;
use Drupal\Core\Form\FormBase;
use Drupal\Core\Render\Markup;
use Drupal\Core\Form\FormStateInterface;
use Symfony\Component\HttpFoundation\RedirectResponse;


/**
 * Defines RawphenotypesManageAssetForm class.
 */
class RawphenotypesManageAssetForm extends FormBase {
  // All default values for trait types.
  private $trait_types;
  // All default values for trait reps.
  private $trait_reps;
  // All units.
  private $trait_units;
  // Controlled vocabularies - types and relationships.
  private $vocabularies;
  // When asset type is project.
  private $project_id;
  // When asset is type trait.
  private $trait_id;

  /**
   * Set default values ie. traits types and trait reps etc.
   */
  public function __construct() {
    $term_service    = \Drupal::service('rawphenotypes.term_service');
    $default_service = \Drupal::service('rawphenotypes.default_service');
      
    $this->vocabularies = $default_service::getDefaultValue('vocabularies');
    $this->trait_types  = $default_service::getTraitTypes();
    $this->trait_reps   = $default_service::getTraitReps();
    $this->trait_units  = $term_service::getTermsByType('phenotype_measurement_units');
  }

  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'rawphenotypes_assets';
  }

  /**
   * {@inheritdoc}
   * Build form.
   */
  public function buildForm(array $form, FormStateInterface $form_state, $asset_id = NULL, $asset_type = NULL, $action = NULL) {
    $term_service = \Drupal::service('rawphenotypes.term_service');
    $user_service = \Drupal::service('rawphenotypes.user_service');

    // Define valid action/command/operation per asset type.
    $arr_valid = [];
    $arr_valid['project']['command'] = ['manage'];
    $arr_valid['project']['table'] = 'pheno_project_cvterm';
    $arr_valid['project']['id'] = 'project_id';

    $arr_valid['header']['command'] = ['edit', 'delete'];
    $arr_valid['header']['table'] = 'pheno_project_cvterm';
    $arr_valid['header']['id'] = 'project_cvterm_id';

    $arr_valid['user']['command'] = ['delete'];
    $arr_valid['user']['table'] = 'pheno_project_user';
    $arr_valid['user']['id'] = 'project_user_id';

    $arr_valid['envdata']['command'] = ['delete'];
    $arr_valid['envdata']['table'] = 'pheno_environment_data';
    $arr_valid['envdata']['id'] = 'environment_data_id';
    
    // Ensure query strings are valid. The string length check ensure that Posgres
    // will not throw Numeric value out of range PDOexception.
    if (!isset($asset_id) || $asset_id <= 0 || strlen($asset_id) >= 10) {
      // When project asset id is invalid.
      $msg = $this->t('Not a valid project asset id number.');
      \Drupal::messenger()->addMessage($msg, 'error');
    }
    else {
        
      

      // Id is valid, test if asset type is valid.
      if (array_key_exists($asset_type, $arr_valid)) {
        // Valid asset type request. Check if the given asset id and asset type
        // exists in the database before peforming any command.
        $sql = sprintf("SELECT FROM {%s} WHERE %s = :asset_id LIMIT 1",
          $arr_valid[ $asset_type ]['table'], $arr_valid[ $asset_type ]['id']);
        
        $args = [':asset_id' => (int)$asset_id];
        $prj_asset = \Drupal::database()
          ->query($sql, $args);

        $prj_asset->allowRowCount = TRUE;

        if ($prj_asset->rowCount() == 1) {
          // Project asset exists in the database. Procede to command requested.
          // Determine what to do with the project asset.
          if (in_array($action, $arr_valid[ $asset_type ]['command'])) {
            // Command is valid. Call function that will execute the command.
            if ($asset_type == 'project') {
              // Call project function.
              $this->project_id = $asset_id;
              $form = $this->projectAssetForm();
            }
            elseif ($asset_type == 'header') {
              // Before admin can modify or delete a column header, ensure that a header
              // is not a plant property type header, not used by another project and the header has no data associated to it.
              // In addition, Loding (scale 1-5) header is not editable.
              $plant_property = $this->trait_types['type4'];
              // Get header properties.
              $header_asset = $term_service::getTermProperties($asset_id, 'full');

              if ($header_asset['count_data'] > 0 && $action == 'delete') {
                // Header has data.
                $msg = $this->t('Cannot @action this entry. Column header has data associated to it.', ['@action' => $action]);
                \Drupal::messenger()->addMessage($msg, 'error');
              }
              else {
                // Call header function.
                $this->trait_id = $asset_id;
                $form = $this->manageHeaders($header_asset, $action);
              }
            }
            else if($asset_type == 'user') {
              // Ensure that user is not deleted when there is data and backup files associated.
              $user_project_asset = $user_service::getUserAssets($asset_id);

              if ($user_project_asset['project_file_count'] > 0) {
                // User is assigned to a project has backup files.
                $msg = $this->t('Cannot @action this user. User has backed up files.', ['@action' => $action]);
                  \Drupal::messenger()->addMessage($msg, 'error');  
              }
              elseif ($user_project_asset['project_data_count'] > 0) {
                // User is assigned to a project has has data.
                $msg = $this->t('Cannot @action this user. User is assigned to a project that has data.', ['@action' => $action]);
                \Drupal::messenger()->addMessage($msg, 'error');
              }
              elseif ($user_project_asset['user_id'] < 2) {
                // User is administrator.
                $msg = $this->t('Cannot @action this user. User is the administrator of thi site.', ['@action' => $action]);
                \Drupal::messenger()->addMessage($msg, 'error');  
              }
              else {
                // Call user function.
                $form = $this->manageUsers($user_project_asset, $action);
              }              
            }
            else if($asset_type == 'envdata') {
              if ($asset_id > 0 && $action == 'delete') {
                // Locate file. Ensure that file to be deleted exists.
                $envdata_project_asset = $project_service::getGetEnvDataAssets($asset_id);
  
                if ($envdata_project_asset) {
                  // Call user function.
                  $form = self::manageEnvData($envdata_project_asset, $action);
                }
                else {
                  // User is administrator.
                  $msg = $this->t('Cannot @action Environment Data File. File does not exist.', ['@action' => $action]);
                  \Drupal::messenger()->addMessage($msg, 'error');    
                }
              }
            }
          }
          else {
            // Not a valid command.
            $msg = $this->t('Not a valid request.');
            \Drupal::messenger()->addMessage($msg, 'error');
          }
        }
        else {
          // Asset record not found.
          $msg = $this->t('Project asset id number does not exist.');
          \Drupal::messenger()->addMessage($msg, 'error');
        }
      }
      else {
        // Asset type does not exist.
        $msg = $this->t('Not a valid project request.');
        \Drupal::messenger()->addMessage($msg, 'error');
      }
    }






    // Attach library.
    $form['#attached']['library'][] = 'rawphenotypes/style-admin';
    $form['#attached']['library'][] = 'rawphenotypes/script-admin';
  
    return $form;
  }


























  /**
   * {@inheritdoc}
   * Validate headers - this method is only for Add/Edit column headers only.
   * Other form in this page uses a custom validate routine.
   */
  public function validateForm(array &$form, FormStateInterface $form_state) {
    $term_service = \Drupal::service('rawphenotypes.term_service');

    // Get the submit button that triggered the submit action. Since this is a general hook_validate()
    // of the entire form, limit the proces only to submit button in from add column header and save column header.
    // When the submit action is determined, perform basic check of ensuring that user is not using cvterm name
    // that is in the database already.
    $btn_submit = $form_state->getTriggeringElement();
    $action = $btn_submit['#name'];

    // Submit buttons has property #name which will be used to determine which among the submit buttons
    // was click and load corresponding action.
    if ($action == 'add-column-header' || $action == 'save') {
      // ADD AND SAVE NEW COLUMN HEADER.
      $fld_name_trait_name = 'txt_trait_name';
      $fld_value_trait_name = trim($form_state->getValue($fld_name_trait_name));

      if (strpbrk($fld_value_trait_name, '()')) {
        // Test if user added parenthesis in the name.
        $form_state->setErrorByName($fld_name_trait_name, 
          $this->t('Characters "(" and/or ")" found in column header name. Please remove these characters and try again.'));
      }
      else {
        $asset_id = $form_state->getValue('txt_id');

        $trait_name = $term_service::constructTerm(
          [
            'name' => $fld_value_trait_name,
            'rep'  => trim($form_state->getValue('sel_trait_rep')),
            'unit' => trim($form_state->getValue('sel_trait_unit'))
          ]
        );

        if ($action == 'add-column-header') {
          // Add:
          // Before adding the cvterm, ensure it is not present in cvterm table.
          $fc = ['name' => $trait_name, 'cv_id' => ['name' => 'phenotype_measurement_types']];
          $found_cvterm = $term_service::getTerm($fc);

          if (isset($found_cvterm->cvterm_id) AND $found_cvterm->cvterm_id > 0) {
            $form_state->setErrorByName($fld_name_trait_name, 
              $this->t('The column header name exists in the database. Please use a different name.'));
            
            $form_state->setErrorByName('sel_trait_rep');
            $form_state->setErrorByName('sel_trait_unit');
          }
        }
        else {
          // Edit: 
          // When renaming a field in edit header, ensure that the new or modified name does not exist in cvterm table.
          $sql = "
            SELECT cvterm_id FROM chado.cv AS t1 INNER JOIN chado.cvterm AS t2 USING(cv_id)
            WHERE t1.name = 'phenotype_measurement_types' AND t2.name = :name AND t2.cvterm_id <> :this_cvterm_id
          ";
          $args = [':name' => $trait_name, ':this_cvterm_id' => $asset_id];
          $found = \Drupal::database()
            ->query($sql, $args);
          
          $found->allowRowCount = TRUE;

          if ($found->rowCount() > 0) {
            $form_state->setErrorByName($fld_name_trait_name, 
              $this->t('The column header name exists in the database. Please use a different name.'));
          
            $form_state->setErrorByName('sel_trait_rep');
            $form_state->setErrorByName('sel_trait_unit');
          }
        }
      }
    }
  }

  /**
   * {@inheritdoc}
   * Submit headers - this method is only for Add/Edit column headers only.
   * Other form in this page uses a custom submit routine.
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    $term_service = \Drupal::service('rawphenotypes.term_service');
    $vocabularies = $this->vocabularies;
    
    // Get the submit button that triggered the submit action. Since this is a general hook_submit()
    // of the entire form, limit the proces only to submit button in from add column header and save column header.
    $btn_submit = $form_state->getTriggeringElement();
    $action = $btn_submit['#name'];

    if ($action == 'add-column-header' || $action == 'save') {
      // Holds the record id number.
      $asset_id = $form_state->getValue('txt_id');

      // Get field values.
      $unit = $form_state->getValue('sel_trait_unit');
      
      // Construct the trait name based on given name, rep and unit.
      $trait_name = $term_service::constructTerm(
        [
          'name' => trim($form_state->getValue('txt_trait_name')),
          'rep'  => trim($form_state->getValue('sel_trait_rep')),
          'unit' => trim($form_state->getValue('sel_trait_unit'))
        ]
      );
      
      // R Friendly version of the Header name.
      // When supplied, use it, otherwise transform the name to R friendly.
      $rver = empty($form_state->getValue('txt_trait_rfriendly'))
        ? $term_service::makeTermRCompatible($trait_name)
        : $form_state->getValue('txt_trait_rfriendly');
    
      $trait_def  = trim($form_state->getValue('txt_trait_def'));
      $col_method = trim($form_state->getValue('txt_trait_method'));
      $trait_type = trim($form_state->getValue('sel_trait_type'));  
      
      // These vocabulary terms are for relationship and property.
      $cv_rver = $term_service::getTerm([
        'name' => $vocabularies['cv_rver'], 
        'cv_id' => ['name' => $vocabularies['cv_phenotypes']]
      ]);
      
      $cv_desc = $term_service::getTerm([
        'name' => $vocabularies['cv_desc'], 
        'cv_id' => ['name' => $vocabularies['cv_phenotypes']]
      ]);

      $cv_unit_rel = $term_service::getTerm([
        'name' => $vocabularies['cv_unit'], 
        'cv_id' => ['name' => $vocabularies['cv_phenotypes']]
      ]);

      // This vocabulary term is for type.
      $cv_unit = $term_service::getTerm(['name' => $vocabularies['cv_unit']], 'cv');
      
      // Get the cvterm id of the unit selected.
      $cvterm_unit = $term_service::getTerm(['name' => $unit, 'cv_id' => $cv_unit->cv_id]);

      if ($action == 'add-column-header') {
        // Add:
        // Uses project id.
        // Insert cvterm.
        $cvterm = $term_service::addTerm(
          [
            'id' => 'rawpheno_tripal:' . $trait_name,
            'name' => $trait_name,
            'cv_name' => 'phenotype_measurement_types',
            'definition' => $trait_def
          ]
        );

        // Add a cvterm prop to store the R friendly version.
        $term_service::saveTermProperty(
          [
            'cvterm_id' => $cvterm->cvterm_id,
            'type_id' => $cv_rver->cvterm_id,
            'value' => $rver,
            'rank' => 0
          ]
        );

        // Add a cvter prop to store the collection method.
        $term_service::saveTermProperty(
          [
            'cvterm_id' => $cvterm->cvterm_id,
            'type_id' => $cv_desc->cvterm_id,
            'value' => $col_method,
            'rank' => 0
          ]
        );

        // Relate the cvterm to unit.
        $term_service::saveTermRelationship(
          [
            'type_id' => $cv_unit_rel->cvterm_id,
            'object_id' => $cvterm->cvterm_id,
            'subject_id' => $cvterm_unit->cvterm_id,
          ]
        ); 
        
        // Add entry to project cvterm table - add trait to project.
        $term_service::saveTermToProject($cvterm->cvterm_id, $trait_type, $asset_id);
      }
      else {
        // Update:
        $term_service::updateTerm($asset_id, $trait_name, $trait_def);
        
        
        // Update rfriendly version in cvtermprop.
        $type_rversion = $term_service::getTerm(
          ['name' => 'phenotype_r_compatible_version'], 
          ['cv_id' => ['name' => 'rawphenotypes_terms']]
        );

        $term_service::updateTermProperty($asset_id, $type_rversion->cvterm_id, $rver);
        
        // Update method of collection.
        // When updating, make sure term has a collection method entry in cvtermprop.
        // If none, add an entry.
        $has_method = $term_service::getTermProperty($asset_id);

        $type_method = $term_service::getTerm(
          ['name' => 'phenotype_collection_method'], 
          ['cv_id' => ['name' => 'rawphenotypes_terms']]
        );

        if (empty($has_method)) {
          // None property for method found, create one.
          $term_service::saveTermProperty(
            [
              'cvterm_id' => $asset_id,
              'type_id' => $type_method->cvterm_id,
              'value' => $col_method,
              'rank' => 0
            ]
          );
        }
        else {
          // Has method property, update record.
          $term_service::updateTermProperty($asset_id, $type_method->cvterm_id, $col_method);          
        }

        // Update relationship term - unit.
        $type_unit = $term_service::getTermByName('phenotype_measurement_units', 'cv');
        
        // Get the unit id.
        $unit_cvterm = $term_service::getTerm(
          [
            'name' => $unit,
            'cv_id' => $type_unit->cv_id
          ]
        );

        $term_service::updateTermRelationship($asset_id, $unit_cvterm->cvterm_id);
        
        // Update trait type.
        // Project id the trait is in.
        $project_id = $form_state->getValue('prj_id');
        $term_service::updateTermProperty($asset_id, 0, $trait_type, $project_id, FALSE);

        // Inform user of the updated header.
        \Drupal::messenger()->addStatus($this->t('You have successfully updated a column header in this project.'));
      }
    }
  }

  /////////
  /**
   * Custom validate - validate selected column headers in Form fieldset #2 - Add existing 
   * column headers.
   */
  public function validateSelectedHeaders(array &$form, FormStateInterface $form_state) {
    // ADD/REUSE EXISTING COLUMN HEADER.
    $btn_submit = $form_state->getTriggeringElement();
    $action = $btn_submit['#name'];
  
    if ($action == 'add-selected-headers') {
      $selected_headers = $form_state->getValue('tbl_existing_headers'); 
  
      if (count(array_filter($selected_headers)) <= 0) {
        $form_state->setErrorByName('tbl_existing_headers', $this->t('No column headers selected.'));
      }
    }
  }

  /**
   * Custom submit - submit selected column headers form in Form fieldset #2 - Add existing 
   * column headers.
   */  
  public function submitSelectedHeaders(array &$form, FormStateInterface $form_state) {
    // ADD/REUSE EXISTING COLUMN HEADER.
    $term_service = \Drupal::service('rawphenotypes.term_service');

    // Uses project id.
    $project_id = $form_state->getValue('txt_id');
    // All Types.
    $types = $this->trait_types;

    // All the headers option - this will be filtered to just
    // the items that were selected.
    $selected_headers = $form_state->getValue('tbl_existing_headers'); 
    // Is essential option - this will contain all the option to
    // set header as essential or not.
    $selected_headers_type = $form_state->getUserInput();

    // Only headers that were selected then inspect the is essential
    // field to determine the type.
    foreach(array_filter($selected_headers) as $m) {
      $key = 'traittype-' . $m;
      $trait_type = ($selected_headers_type[ $key ]) ? $types['type1'] : $types['type2'];      
      
      $cvterm_id = (int)$m;
      $term_service::saveTermToProject($cvterm_id, $trait_type, $project_id);
    }
    
    // Inform user of the newly added headers.
    \Drupal::messenger()->addStatus($this->t('You have successfully added a column header to this project.'));
  }
  
  /**
   * Custom validate - validate add user to project.
   */
  public function validateUser(array &$form, FormStateInterface $form_state) {
    $user_service = \Drupal::service('rawphenotypes.user_service');
    $project_service = \Drupal::service('rawphenotypes.project_service');

    $project_id = $form_state->getValue('txt_id');
    $user = $form_state->getValue('txt_autocomplete_user');
  
    if (empty($user)) {
      $form_state->setErrorByName('txt_autocomplete_user', $this->t('No user name supplied in the field.'));
    }
    else {
      // Search user
      $user_id = $user_service::getUserIdByUsername($user);

      if (empty($user_id)) {
        $form_state->setErrorByName('txt_autocomplete_user', $this->t('User does not exist.'));
      }
      else {
        // Test if user was added twice in the same project.
        $project_users = $project_service::getProjectActiveUsers($project_id);
        
        foreach($project_users as $u) {
          if ($u->uid == $user_id) {
            $form_state->setErrorByName('txt_autocomplete_user', 
              $this->t('User is already active in this project and cannot be added again.'));
            
            break;
          }
        }
      }
    }
  }

  /**
   * Custom submit - submit add user form.
   */
  public function submitUser(array &$form, FormStateInterface $form_state) {
    $user_service = \Drupal::service('rawphenotypes.user_service');
    $project_service = \Drupal::service('rawphenotypes.project_service');

    // Uses project id
    $project_id = $form_state->getValue('txt_id');
    $user = $form_state->getValue('txt_autocomplete_user');
    $user_id = $user_service::getUserIdByUsername($user);

    $project_service::addUserToProject($user_id, $project_id);
    \Drupal::messenger()->addStatus($this->t('You have successfully added a user to this project.'));
  }

   
  /**
   * 
   */
  public function submitEnvData(array &$form, FormStateInterface $form_state) {
    $envdata_service = \Drupal::service('rawphenotypes.envdata_service');
    
    $project_id = $form_state->getValue('txt_id');
    $location   = $form_state->getValue('select_location');
    $year       = $form_state->getValue('select_year');

    if ($project_id && $location && $year) {
      $get_field = $this->getRequest()->files->get('files', []);
      $file_field = $get_field['file_env_file'];

      $destination = \Drupal::config('system.file')
        ->get('default_scheme') . '://rawphenotypes_env_data';
      
      \Drupal::service('file_system')
        ->prepareDirectory($destination, FILE_MODIFY_PERMISSIONS);
      
      $env_filename = $file_field->getClientOriginalName();
      $env_file_ext = $file_field->getClientOriginalExtension();

      $seq_no = $envdata_service::getSequenceNumber($project_id, $location, $year);
      $u_no = date('ymdis');

      $new_filename = str_replace(array(' ', '-', ','), '_', $location) . '_' . $year . '_' . $u_no . '_environment_data.' . $env_file_ext;
      $new_filename = strtolower($new_filename);
      
      $file_field->move($destination, $new_filename);
      
      // Create a Drupal 8 file entity.
      $file_create = File::create([
        'filename' => $new_filename,
        'uri' => $destination . '/' . $new_filename,
        'status' => 1
      ]);
      
      $file_create->save();

      $fid = $file_create->id();

      $envdata_service::saveEnvData(
        [
          'project_id' => $project_id,
          'fid'       => $fid,
          'location' => $location,
          'year'    => $year,
          'sequence_no' => $seq_no
        ]
      );
          
      \Drupal::messenger()->addStatus($this->t('You have successfully uploaded environment data file to this project.'));  
    }
  }

  /**
   * 
   */
  public function validateEnvData(array &$form, FormStateInterface $form_state) {
    // Environment Data File:
    $field_value = $this->getRequest()->files->get('files', []);

    // File field:
    if ($field_value['file_env_file'] == NULL) {
      $form_state->setErrorByName('file_env_file', $this->t('Environment Data File is empty. Please select a file and try again.'));
    }

    // Location:
    if (empty($form_state->getValue('select_location'))) {
      $form_state->setErrorByName('select_location', $this->t('Location field is empty. Please select an option and try again.'));
    }

    // Year:
    if (empty($form_state->getValue('select_year'))) {
      $form_state->setErrorByName('select_year', $this->t('Year field is empty. Please select an option and try again.'));
    }    
  }



















  
  // FORMS:


  /**
   * Main project asset page. Show all assets in tabs namely
   * column headers, users and environment data.
   * 
   * This page will also give user to add new items to individual
   * project asset types.
   * 
   * Property - project_id is set in the switch and will be used
   * in this method.
   */
  public function projectAssetForm() {
    $form = [];
    // Load services.
    $term_service    = \Drupal::service('rawphenotypes.term_service');
    $user_service    = \Drupal::service('rawphenotypes.user_service');
    $project_service = \Drupal::service('rawphenotypes.project_service');
    
    // Get project.
    $project_id = $this->project_id;
    $project = $project_service::getProject($project_id);

    // Trait Types.
    $trait_types = $this->trait_types;

    $form['project_name'] = [
      '#type' => 'inline_template',
      '#template' => '<h2>PROJECT: ' . $project->name . '</h2>'
    ];

    // Construct page. Layout will be forms (add headers, user and env. data) followed
    // by summary (in tables) of all assets organized into tabs.
    
    // FORMS:
    // Layout forms.

    // Add new column headers.
    // Fieldset add column header form.
    $form['fieldset_trait'] = [
      '#type' => 'details',
      '#title' => $this->t('Add column header'),
      '#open' => FALSE,
    ];
      
      $default = [
        'count_project'    => 0,
        'count_data'       => 0,
        'txt_id'           => $project_id,
        'btn_trait_submit' => 'Add column header'
      ];

      $new_header_elements = $this->newHeaderForm($default);
      $form['fieldset_trait'][] = $new_header_elements;
    
    // Add/Reuse headers.
    // Fieldset suggest existing column headers.
    $form['fieldset_existing_trait'] = [
      '#type' => 'details',
      '#title' => $this->t('Add existing column headers'),
      '#open' => FALSE,
    ];

        $default = [
          'txt_id' => $project_id,
        ];

        $existing_header_elements = $this->existingHeaderForm($default);
        $form['fieldset_existing_trait'][] = $existing_header_elements;

    // Add users (data collector accounts) to the project.
    // Fieldset to suggest user to a project.
    $form['fieldset_users'] = [
      '#type' => 'details',
      '#title' => $this->t('Add users'),
      '#open' => FALSE,
    ];  
      
      $user_elements = $this->userForm();
      $form['fieldset_users'][] = $user_elements;

    // Manage environment data.  
    // Construct form to manage Environment data.
    $form['fieldset_envdata'] = [
      '#type' => 'details',
      '#title' => $this->t('Upload Environment Data File'),
      '#open' => FALSE,
    ];
  
      $env_elements = $this->environmentDataForm();
      $form['fieldset_envdata'][] = $env_elements;  


    // TABLES:
    // Layout summary tables.
    
    // List column headers in a project.
    // Given a project id, select all column headers belonging to that project.
    // Note the loding (scale 1-5) upright-lodged header is the only header using this
    // format (all headers use the format: name (trait rep; unit), thus a flag indicating
    // if header is such is required to disable edit option and prevent edit functionality
    // from processing unfamiliar format.
    $headers = $project_service::getProjectTerms($project_id);
    $users   = $project_service::getProjectActiveUsers($project_id);
    $envdata = $project_service::getProjectEnvDataFiles($project_id);

    // Array to hold table headers.
    $arr_headers = [];

    // Array to hold table rows.
    $arr_rows = [];

    // Warn admin that project has no essential trait
    foreach($headers as $h) {
      if ($h->type == $trait_types['type1']) $count_essential++;
    }

    if ($count_essential < 1) {
      $form['message_no_essential'] = [
        '#type' => 'inline_template',
        '#theme' => 'theme-rawphenotypes-message',
        '#data' => [
          'message' => $this->t('Project has no Essential Column Header.'),
          'type' => 'warning'
        ]
      ];
    }

    // Warn admin that project has no assigned user.
    if (count($users) < 1) {
      $form['message_no_user'] = [
        '#type' => 'inline_template',
        '#theme' => 'theme-rawphenotypes-message',
        '#data' => [
          'message' => $this->t('Project has no active users.'),
          'type' => 'warning'
        ]
      ];
    }

    // Tabs to show either the project column headers table or project active users table.
    // Add 1 to account for Name column header.
    $form['nav_tabs'] = [
      '#type' => 'inline_template',
      '#template' => '
        <div id="nav-tabs">
          <ul>
            <li class="active-tab">' . ((int)count($headers) + 1). ' Column Headers</li>
            <li>' . count($users) . ' Active Users</li>
            <li>' . count($envdata) . ' Environment Data File</li>
          </ul>
        </div>
      '
    ];
    

    // Table column headers.
    // Construct table that lists all column headers specific to a project.
    // Options to edit and delete items give admin record management functionality.
    // NOTE: Delete option will not physically delete a record in cvterm table.
    //       When a header is deleted, it is removed from the project only.

    $has_name = 0;
    if (count($headers) > 0) {
      $i = 0;

      foreach($headers as $h) {
        // Get header information.
        $header_asset = $term_service::getTermProperties($h->project_cvterm_id); 
        
        $edit_cell = '-';
        if ($header_asset['name'] != 'Lodging (Scale: 1-5) upright - lodged') {
          $link = Url::fromRoute('rawphenotypes.manage_project', ['asset_id' => $h->project_cvterm_id, 'asset_type' => 'header', 'action' => 'edit']);
          $edit_cell = \Drupal::l($this->t('Edit'), $link);
        }

        // Delete link.
        $link = Url::fromRoute('rawphenotypes.manage_project', ['asset_id' => $h->project_cvterm_id, 'asset_type' => 'header', 'action' => 'delete']);
        $del_cell = \Drupal::l($this->t('Remove'), $link);

        // No edit and delete when column header is of type plantproperty.
        if ($header_asset['type'] == $trait_types['type4'] || $header_asset['name'] == 'Planting Date (date)') {
          // Add Name column header to the row array.
          if ($has_name == 0 AND $header_asset['name'] != 'Planting Date (date)') {
            $markup_name = Markup::create('<h4>Name</h4>');
            array_push($arr_rows, array((count($arr_rows) + 1), $markup_name, 'Name', 'PLANTPROPERTY', '-', '-'));
            $has_name += 1;
            $i++;
          }

          $edit_cell = $del_cell = '-';
        }
        
        $class = ($header_asset['type'] == $trait_types['type1']) ? 'essential-trait' : 'non-essentialtrait';

        if ($header_asset['type'] == $trait_types['type4']) {
          // Row where trait is plantproperty.
          $header_cell = '<h4>' . $header_asset['name'] . '</h4>';
          $definition_cell = $header_asset['name'];
        }
        else {
          // All traits.
          $header_cell = '<h4>' . $header_asset['name'] . '</h4><p class="r-ver">' . self::markEmpty($header_asset['r_version']) . '</p>';
          $definition_cell = '<p><strong>DEFINITION:</strong><br /> ' . @self::markEmpty($header_asset['definition']) .  '</p>' .
                             '<p><strong>COLLECTION METHOD:</strong><br />' . @self::markEmpty($header_asset['method']) . '</p>';
          
          $header_cell = $header_cell;
          $definition_cell = $definition_cell;
        }

        // Register a row.
        $arr_rows[] = [
          ($i+1),                            // Row counter.
          Markup::create($header_cell),      // Trait/header name.
          Markup::create($definition_cell),  // Trait/header definition.
          [
            'data' => strtoupper($header_asset['type']),
            'class' => [$class]
          ],                                 // Add class to cell to highlight text when trait is essential trait.
          Markup::create($edit_cell),        // A link to edit a trait.
          Markup::create($del_cell)          // A link to delete a trait.
        ];

        $i++;
      }
    }
    
    array_push($arr_headers, '-', $this->t('Column Header <small>(unit)</small>'), $this->t('Definition/Collection Method'), $this->t('Type'), $this->t('Edit'), $this->t('Remove'));
    $form['tbl_project_headers'] = [
      '#type' => 'table',
      '#title' => $this->t('Projects'),
      '#header' => $arr_headers,
      '#rows' => $arr_rows,
      '#empty' => $this->t('No column headers in this project'),
      '#prefix' => '<div id="container-prj-hdr" class="container-table-data tab-data">',
      '#suffix' => '</div>',
      '#attributes' => ['id' => 'tbl-project-headers']
    ];
    
    // Table active users.
    // Construct table that lists all active users to a project along with files
    // associated to a user in a given project.
    // Options to delete items give admin record management functionality.
    // NOTE: Delete option will not physically delete a record in users table.
    //       When a user is deleted, it is removed from the project only.

    $arr_rows = $arr_headers = [];
    if (count($users) > 0) {
      $i = 0;
      
      foreach($users as $p_uid => $u) {
        if (empty($u->name)) continue;

        // Delete link.
        $link = Url::fromRoute('rawphenotypes.manage_project', ['asset_id' => $p_uid, 'asset_type' => 'user', 'action' => 'delete']);
        $del_cell = \Drupal::l($this->t('Remove'), $link);

        // Create a row of user with other relevant user info.
        $cell_user_name = Markup::create('<div class="tag-name">' . $u->name . '</div>');
        $cell_last_login = Markup::create('<em>' . format_date($u->login) . '</em>'); 

        $arr_rows[] = [
          ($i+1),            // Row number.
          $cell_user_name,   // Name of user.
          $u->mail,          // Email address.
          $cell_last_login,  // Date of last login.
          $del_cell          // Link to remove user from a project.
        ];
        
        $result_F = $user_service::getUserFiles($p_uid);
        $my_files_count = count($result_F);

        $arr_header_F = ['File', 'Version', 'Archive', 'Notes', 'Validation Result'];
        $arr_rows_F = [];

        if ($my_files_count > 0) {
          foreach($result_F as $f) {
            // File.
            $link = Url::fromUri($f->uri);
            $cell_file = \Drupal::l($f->filename . ' (' . format_size($f->filesize) . ')', $link, ['attributes' => ['target' => '_blank']]);
            $cell_file .= Markup::create('<br /><small>Uploaded: ' . \Drupal::service('date.formatter')->format($f->timestamp) . '</small>');

            // Is archive?
            $cell_is_archive = ($f->archive == 'y') ? 'Yes' : 'No';

            // Notes.
            $cell_notes = (empty($f->notes))
              ? '&nbsp;'
              : Markup::create('<div class="container-cell" title="Click to expand or collapse" id="vn-file-' . $f->file_id . '">' . $f->notes . '</div>');

            $vr_tmp = str_replace(['#item: (passed)', '#item: (failed)'],
              ['<p class="pass"> (PASSED)', '<p class="fail"> * (FAILED)'],
              $f->validation_result) . '</p>';
            
            $vr_tmp = Markup::create($vr_tmp);

            $alert = '';
            if (($n = substr_count($vr_tmp, 'FAILED')) > 0) {
              $alert = Markup::create('<span class="alert-error" title="File has failed validation notices...">' . $n . ' Validation errors</span>');
            }

            $cell_validation = $alert . '<div class="container-cell" id="vr-file-' . $f->file_id . '" title="Click to expand or collapse">'. $vr_tmp .'</div>';
            $cell_validation = Markup::create($cell_validation);

            // Create row showing backed up file with file description.
            $arr_rows_F[] = [
              $cell_file,        // Filename.
              '#' . $f->version, // File version.
              $cell_is_archive,  // Indicate if file is archived or not.
              $cell_notes,       // Notes, comments to file.
              $cell_validation   // Validation result performed to the file.
            ];
          }
        }
        
        $file_table = [
          '#type' => 'table',
          '#title' => 'Files',
          '#header' => $arr_header_F,
          '#rows' => $arr_rows_F,
          '#empty' => $this->t('0 Files'),
          '#attributes' => ['id' => 'tbl-my-files']
        ];

        $link_show_folder = ($my_files_count > 0)
        ? Markup::create('<a href="#" id="folder-' . $p_uid . '" class="link-show-folder">[Show]</a>')
        : '';

        // Account status.
        $acc_status = ($u->status == 1) ? 'Active' : 'Suspended';

        $markup = sprintf('
          <span><strong>[ACCOUNT]</strong> Status: %s | Created: %s - %d Files uploaded %s</span>
          <div id="show-folder-'. $p_uid .'" class="div-my-folder">
            <div class="container-my-folder">%s</div>
          </div>', $acc_status, \Drupal::service('date.formatter')->format($u->created), $my_files_count, $link_show_folder, $file_table);
     
        // Create markup (container for table of files) to show user backed up files.
        $arr_rows[] = [
          'data' => [
            [
              'data' => Markup::create($markup),
              'colspan' => 5,
              'class' => 'row-user-my-folder',
            ]
          ]
        ];
        
        $i++;
      }
    }
    
    array_push($arr_headers, '-', $this->t('Name'), $this->t('Email Address'), $this->t('Last Login'), $this->t('Remove'));
    $form['tbl_project_users'] = [
      '#type' => 'table',
      '#title' => $this->t('User'),
      '#header' => $arr_headers,
      '#rows' => $arr_rows,
      '#empty' => $this->t('No users in this project'),
      '#prefix' => '<div id="container-prj-usr" class="container-table-data tab-data">',
      '#suffix' => '</div>',
      '#attributes' => ['id' => 'tbl-project-users']
    ];

    // Table environment data.
    $arr_rows = $arr_headers = [];

    if (count($envdata) > 0) {
      array_push($arr_headers, '-', t('File'), t('Location'), t('Year'), t('Sequence No.'), t('Delete'));

      foreach($envdata as $i => $env) {
        $link = Url::fromUri($env->uri);
        $cell_file = \Drupal::l($env->filename . ' (' . format_size($env->filesize) . ')', $link, ['attributes' => ['target' => '_blank']]);
        
        $cell_file .= Markup::create('<br /><small>Uploaded: ' . \Drupal::service('date.formatter')->format($env->timestamp) . '</small>');

        $link = Url::fromRoute('rawphenotypes.manage_project', ['asset_id' => $env->environment_data_id, 'asset_type' => 'envdata', 'action' => 'delete']);
        $cell_del = \Drupal::l($this->t('Delete'), $link);

        $arr_rows[] = array($i+1, $cell_file, $env->location, $env->year, '#' . $env->sequence_no, $cell_del);
      }
    }

    $empty_table_title = $this->t('No environment data in this project.');
    $form['tbl_project_envdata'] = [
      '#type' => 'table',
      '#title' => $this->t('Environment Data'),
      '#header' => $arr_headers,
      '#rows' => $arr_rows,
      '#empty' => $empty_table_title,
      '#prefix' => '<div id="container-prj-env" class="container-table-data tab-data">',
      '#suffix' => '</div>',
      '#attributes' => ['id' => 'tbl-project-envdata']
    ];

    return $form;
  }



  /////////////////////////////////
  /**
   * 
   */
  public function manageEnvData($envdata_project_asset, $action) {
    if ($action == 'delete') {

    }
  }

  /**
   * 
   */
  public function manageUsers($user_assets, $action) {
     $user_service = \Drupal::service('rawphenotypes.user_service');

     if ($action == 'delete') {
      $user_service::removeUserFromProject($user_asset['project_user_id']); 

      $link = Url::fromRoute('rawphenotypes.manage_project', 
        ['asset_id' => $user_asset['project_id'], 'asset_type' => 'project', 'action' => 'manage']);
      
      $redirect = new RedirectResponse($link->toString());
      $redirect->send();

      return null;
     }
  }

  /**
   * Manage update and delete of column headers.
   * 
   * Property - trait id is set in the switch and will be used
   * in this method.
   */
  public function manageHeaders($header_asset, $action) {
    $term_service = \Drupal::service('rawphenotypes.term_service');

    $form['project_name'] = [
      '#type' => 'inline_template',
      '#template' => '<h2>Manage Project Assets / Update Header</h2>'
    ];
    
    if ($action == 'edit') {
      if ($header_asset['name'] == 'Lodging (Scale: 1-5) upright - lodged' || $header_asset['name'] == 'Comments') {
        // Is lodging header.
        $goback = Markup::create('&nbsp;<a href="javascript:history.back();">Go back</a>');
        drupal_set_message($this->t('Cannot edit @name.', array('@name' => $header_asset['name'])) . $goback, 'error');
      }
      else {
        // Other headers.
        // CONSTRUCT EDIT FORM.

        // FORM
        // Add a link to allow administrator to go back to the list of traits in a project.
        $link = Url::fromRoute('rawphenotypes.projects', []);
        $link_project = \Drupal::l($this->t('Go back to projects tables'), $link);

        $link = Url::fromRoute('rawphenotypes.manage_project', ['asset_id' => $header_asset['in_project_id'], 'asset_type' => 'project', 'action' => 'manage']);
        $link_manage_asset = \Drupal::l($this->t('Go back to project column headers table'), $link);
        
        $form['back_link'] = [
          '#type' => 'inline_template',
          '#template' => Markup::create($link_project . ' | ' . $link_manage_asset),
        ];

        $form['fieldset_trait'] = [
          '#type' => 'details',
          '#title' => $this->t('Eidt Column Header:'),
          '#open' => TRUE,
        ];

        // Trait name field.
        // Extract the trait rep value, trait unit and the trait name.
        // NOTE: this can only process header following the format: trait name (trait rep; unit)
        $trait_rep = $trait_unit = '';
        // Extract the text value inside the parenthesis (unit part).
        $t = preg_match("/.*\(([^)]*)\)/", $header_asset['name'], $match);
        $u = (isset($match[1])) ? $match[1] : '';

        // Extract information in a unit only when there is a unit in the first place.
        // Get the Trait rep (eg R1, R7, 1st) and measurement unit (eg cm, days) values.
        $reps = $this->trait_reps;

        if ($t) {
          // Split the information and see if either trait rep or unit is present.
          $p = explode(' ', $u);
          if (count($p) > 1) {
            // Has trait rep and unit.
            list($trait_rep, $trait_unit) = $p;
          }
          else {
            if (in_array(trim($u, ';'), $reps)) {
              // Just the rep no unit.
              $trait_rep = $u;
              $trait_unit = '';
            }
            else {
              // Just the unit no trait rep.
              $trait_rep = '';
              $trait_unit = $u;
            }
          }
        }

        // When niether is present, use the default value of the trait rep and unit (null).
        // Trait name without the unit part.
        $trait_name = preg_replace('/\(.*/', ' ', $header_asset['name']);

        // Add form elements.
        // Include the project id when modifying a trait header.
        $default_values = [
          'count_project'        => $header_asset['count_project'],
          'count_data'           => $header_asset['count_data'],
          'txt_id'               => $header_asset['cvterm_id'],
          'prj_id'               => $header_asset['in_project_id'],
          'txt_trait_name'       => trim($trait_name),
          'sel_trait_rep'        => trim($trait_rep),
          'sel_trait_unit'       => trim($trait_unit),
          'txt_trait_definition' => $header_asset['definition'],
          'txt_trait_method'     => $header_asset['method'],
          'txt_trait_rfriendly'  => $header_asset['r_version'],
          'sel_trait_type'       => $header_asset['type'],
          'btn_trait_submit'     => 'Save'
        ];

        $form['fieldset_trait'][] = $this->newHeaderForm($default_values);

        // TABLE
        // Construct a table, as a summary, showing information about the header.
        $markup = '<h2>SUMMARY: ' . $header_asset['name'] . ' : PROJECT: ' . $header_asset['in_project_name'] . '</h2>';
        $form['header'] = [
          '#type' => 'inline_template',
          '#template' => $markup
        ];

        $header_cell = Markup::create($header_asset['name'] . '<p class="r-ver">' . self::markEmpty($header_asset['r_version']) . '</p>');        
        $arr_headers = ['-', $this->t('Column Header <small>(unit)</small>'), $this->t('Collection Method'), $this->t('Definition'), $this->t('Type')];
        $arr_rows[] = [
          1,
          Markup::create($header_cell),
          $header_asset['method'],
          $header_asset['definition'],
          strtoupper($header_asset['type'])
        ];
        
        $form['tbl_project_headers'] = [
          '#type' => 'table',
          '#title' => $this->t('Header summary'),
          '#header' => $arr_headers,
          '#rows' => $arr_rows,
          '#empty' => $this->t('Header summary not found'),
          '#attributes' => ['id' => 'tbl_header_summary']
        ];
      }
    }
    elseif ($action == 'delete') {
      $term_service::removeTermFromProject($header_asset['cvterm_id'], $header_asset['in_project_id']); 

      $link = Url::fromRoute('rawphenotypes.manage_project', 
        ['asset_id' => $header_asset['in_project_id'], 'asset_type' => 'project', 'action' => 'manage']);
      
      $redirect = new RedirectResponse($link->toString());
      $redirect->send();

      return null;
    }

    return $form;
  } 

  /**
   * Add or update new header form.
   */
  public function newHeaderForm($default) {
    $a = $this->trait_types;

    // In this trait types array, remove the plant property option.
    // Allow only to add essential, optional, contributed or subset column header types.
    unset($a['type4']);

    // Type is no user contributed. Do not suggest contributed so user will have not option to add contributed
    // trait using admin since contributed traits are traits derived from submitted spreadsheet in stage 01 : Describe New Trait.
    if (!isset($default['sel_trait_type']) || $default['sel_trait_type'] != $a['type5']) {
      unset($a['type5']);
    }

    // Get the values and use it as both key and value.
    $t = array_values($a);
    $option_trait_type = array_combine($t, array_map('strtoupper', $t));

    // Determine if the header has data or is used in another project.
    if ($default['count_data'] > 0 OR $default['count_project'] > 0) {
      $form['fieldset_trait']['warning'] = [
        '#type' => 'inline_template',
        '#theme' => 'theme-rawphenotypes-message',
        '#data' => [
          'message' => $this->t('This column header has data associated to it or is used in another project.'),
          'type' => 'warning'
        ]
      ];

      $disabled = TRUE;
    }
    else {
      $disabled = FALSE;
    }

    // Project id the trait is in or the trait id number.
    $form['txt_id'] = [
      '#type' => 'hidden',
      '#value' => $default['txt_id'],
    ];

    // Exclusive to modifying trait, include the project id the trait is registered.
    if (isset($default['prj_id']) && $default['prj_id'] > 0) {
      $form['fieldset_trait']['prj_id'] = [
        '#type' => 'hidden',
        '#value' => $default['prj_id'],
      ];
    }

    // Trait name field.
    $form['fieldset_trait']['txt_trait_name'] = [
      '#type' => 'textfield',
      '#title' => $this->t('Name:'),
      '#description' => $this->t('A Concise human-readable name or label for the column header'),
      '#default_value' => isset($default['txt_trait_name']) ? $default['txt_trait_name'] : '',
      '#disabled' => $disabled,
      '#required' => TRUE,
    ];

    // Trait rep field. Default to none.
    // Note: Trait Rep to call 1st, 2nd in unit is not final.
    // Note: The list might change in the future.
    $reps = $this->trait_reps;
    $reps_val = [];
    foreach($reps as $r) {
      $reps_val[] = $r . ';';
    }
    $trait_rep = ['' => 'None'] + array_combine($reps_val, $reps);
    
    $form['fieldset_trait']['sel_trait_rep'] = [
      '#type' => 'select',
      '#title' => $this->t('Trait Rep/Stages:'),
      '#options' => $trait_rep,
      '#default_value' => isset($default['sel_trait_rep']) ? $default['sel_trait_rep'] : reset($trait_rep),
      '#disabled' => $disabled,
    ];

    // Trait unit field. Default to none.
    // Query available units in chado cvterm of type phenotype_measurement_unit.
    $unit = $this->trait_units;

    // Default to text unit.
    $unit_keys = array_keys($unit);
    $textunit = array_search('text', $unit_keys);
    $textunit_id = $unit_keys[$textunit];

    $form['fieldset_trait']['sel_trait_unit'] = [
      '#type' => 'select',
      '#title' => $this->t('Unit:'),
      '#options' => $unit,
      '#default_value' => isset($default['sel_trait_unit']) ? $default['sel_trait_unit'] : $textunit_id,
      '#disabled' => $disabled,
    ];

    // R Friendly field.
    $r_version = isset($default['txt_trait_rfriendly']) ? ' (' . $default['txt_trait_rfriendly'] . ')' : '';

    $form['fieldset_trait']['txt_trait_rfriendly'] = [
      '#type' => 'textfield',
      '#title' => $this->t('R Friendly:@r_version', ['@r_version' => $r_version]),
      '#description' => $this->t('Leave this field blank to let system generate R Friendly version'),
      '#default_value' => '',
      '#disabled' => FALSE,
    ];

    if (isset($default['txt_trait_rfriendly']) && !empty($default['txt_trait_rfriendly'])) {
      // When editing header, add this hidden field containing the original R version value prior to saving.
      // When user decides to provide an alternative r version then save it, otherwise save the value of this
      // hidden field.
      $form['fieldset_trait']['txt_trait_rfriendly_val'] = [
        '#type' => 'hidden',
        '#default_value' => $default['txt_trait_rfriendly'],
      ];
    }

    // Trait definition field.
    $form['fieldset_trait']['txt_trait_def'] = [
      '#type' => 'textarea',
      '#title' => $this->t('Definition:'),
      '#description' => $this->t('A human-readable text definition'),
      '#required' => TRUE,
      '#default_value' => isset($default['txt_trait_definition']) ? $default['txt_trait_definition'] : '',
      '#disabled' => $disabled,
    ];

    // Describe method of collection field.
    $form['fieldset_trait']['txt_trait_method'] = [
      '#type' => 'textarea',
      '#title' => $this->t('Describe Method:'),
      '#description' => $this->t('Describe the method used to collect this data if you used a scale, be specific'),
      '#required' => TRUE,
      '#default_value' => isset($default['txt_trait_method']) ? $default['txt_trait_method'] : '',
      '#disabled' => $disabled,
    ];

    // Tell user about contributed trait when detected. Such trait is not included when generating data collection
    // spreadsheet file - to incorporate it, please set the trait type to either essential or optional.
    if (isset($a['type5']) && isset($default['sel_trait_type']) && $default['sel_trait_type'] == $a['type5']) {
      // The header is contributed. Indicate to user as such.
      $form['fieldset_trait']['warning_contributed'] = [
        '#type' => 'inline_template',
        '#theme' => 'theme-rawphenotypes-message',
        '#data' => [
          '#message' => $this->t('This column header is a user contributed trait and is not incorporated in 
            generating Data Collection Spreadsheet file for this Project. To include this trait, 
            set the trait type to Essential or Optional.'),
          '#type' => 'warning'
        ]
      ];
    }

    // Trait is essential field. Default to unchecked.
    $form['sel_trait_type'] = [
      '#type' => 'select',
      '#title' => $this->t('Type:'),
      '#description' => $this->t('Set the type to <span class="essential-trait">ESSENTIAL</span> to ensure this header must exists in the spreadsheet file'),
      '#options' => $option_trait_type,
      '#default_value' => isset($default['sel_trait_type']) ? $default['sel_trait_type'] : reset($option_trait_type),
      '#disabled' => FALSE,
    ];

    // Save trait button.
    // #name key property is used to refer to later as a triggering element.
    $btn_name = str_replace(' ', '-', strtolower($default['btn_trait_submit']));
    $form['btn_trait_subtmit'] = [
      '#type' => 'submit',
      '#name' => $btn_name,
      '#value' => $this->t('@save_or_add', ['@save_or_add' => $default['btn_trait_submit']]),
      '#suffix' => '<span>&nbsp;* means field is required</span>',
    ];
    
    return $form;
  }

  /**
   * Add or reuse existing header form.
   */
  public function existingHeaderForm($default) {
    $term_service    = \Drupal::service('rawphenotypes.term_service');

    // Table rows.
    $arr_tblchkbox_rows = [];
    $headers = $term_service::getTermsNotInProject($default['txt_id']);

    if (count($headers) > 0) {
      foreach($headers as $h) {
        $arr_tblchkbox_rows[$h->cvterm_id] =
          [
            'name' => $h->name,
            // Add select box.
            'traittype' =>
              ['data' =>
                [
                  '#type' => 'checkbox',
                  '#title' => 'Yes',
                  '#options' => [0, 1],
                  '#default_value' => 0,
                  '#name' => 'traittype-' . $h->cvterm_id
                ]
              ]                                        
          ];
      }
    }

    // Table headers.
    $arr_tblchkbox_headers = ['traittype' => $this->t('Is Essential?'), 'name' => $this->t('Name')];

    // Checkboxes and table.
    $form['tbl_existing_headers'] = [
      '#type' => 'tableselect',
      '#header' => $arr_tblchkbox_headers,
      '#options' => $arr_tblchkbox_rows,
      '#js_select' => FALSE,
      '#prefix' => '<p>' .
        $this->t('The table below lists all column headers available in this module.
        Please check the header(s) that you want to add to this project and click Add Selected Headers button.') . '</p>
        <div class="container-table-data table-data">',
      '#suffix' => '</div><p>' .
        $this->t('Check <span class="essential-trait">IS ESSENTIAL?</span> to ensure the header must exists in the spreadsheet file')
        . '</p>',
      '#empty' => t('No column headers available'),
      '#attributes' => [
        'id' => 'tbl-existing-headers',
        'class' => [
          'tableheader-processed'
        ],
      ],
      '#theme_wrappers' => []
    ];

    // Add submit button only there is any options available.
    // #name key property is used to refer to later as a triggering element.
    if (count($arr_tblchkbox_rows) > 0) {
      $form['add_selected_trait'] = [
        '#type' => 'submit',
        '#value' => $this->t('Add selected headers'),
        '#name' => 'add-selected-headers',
        '#submit' => ['::submitSelectedHeaders'],
        '#validate' => ['::validateSelectedHeaders'],
        '#limit_validation_errors' => [
          ['tbl_existing_headers'],
          ['txt_id']
        ],
      ];
    }

    return $form;
  }

  /**
   * Add data collector user to project.
   */
  public function userForm() {
    // Autocomplet search field.
    // Add seach field to filter the list of name.
    $form['txt_autocomplete_user'] = [
      '#title' => $this->t('Name :'),
      '#type' => 'textfield',
      '#maxlength' => 50,
      '#size' => 130,
      '#autocomplete_route_name' => 'rawphenotypes.autocomplete.user',
      '#description' => $this->t('Type the name or username of the user'),
    ];

    $form['add_selected_user'] = [
      '#type' => 'submit',
      '#value' => $this->t('Add user'),      
      '#submit' => ['::submitUser'],
      '#validate' => ['::validateUser'],
      '#limit_validation_errors' => [
        ['txt_autocomplete_user'],
        ['txt_id']
      ],
    ]; 

    return $form;
  }

  /**
   * Manage environment data asset form.
   */
  public function environmentDataForm() {
    // Environement Data File Upload.
    $inline_wraper = [
      '#prefix' => '<div class="envdata-field-wrapper">',
      '#suffix' => '</div>',
    ];

    // Add seach field to filter the list of name.
    $form['file_env_file'] = [
      '#title' => $this->t('File :'),
      '#type' => 'file',
    ];

    //$location_options = $project_service::getProjectLocations($project_id);

    $form['select_location'] = [
      '#title' => $this->t('Location :'),
      '#type' => 'select',
      '#options' => ['canada' => 'Canada'],
      '#empty_option' => $this->t('- Select -'),
    ];

    $form['select_year'] = [
      '#title' => $this->t('Year :'),
      '#type' => 'select',
      '#options' => ['2001' => '2001'],
      '#empty_option' => $this->t('- Select -'),
    ];

    $form['upload_env_file'] = [
      '#type' => 'submit',
      '#submit' => ['::submitEnvData'],
      '#validate' => ['::validateEnvData'],
      '#limit_validation_errors' => [
        ['file_env_file'],
        ['select_location'],
        ['select_year'],
        ['txt_id']
      ],
      '#value' => $this->t('Upload file')
    ];

    return $form;
  }

  /**
   * Function set empty value.
   */
  public static function markEmpty($val) {
    return $val ?? '-';
  }
}