<?php
/**
 * @file 
 * Create project-specific data collection spreadsheet file.
 * Accessible through the instructions page.
 */

namespace Drupal\rawphenotypes\Controller;

use Drupal\Core\Controller\ControllerBase;
use Symfony\Component\HttpFoundation\BinaryFileResponse;

/**
 * Defines RawphenotypesCreateProjectSpreadsheet class.
 */
class RawphenotypesCreateProjectSpreadsheet extends ControllerBase {
  // Project services.
  private $project_service;
  // Term services.
  private $term_service;
  // Default values.
  private $default_service;
  // User service.
  private $user_service;

  /**
   * Initialize services.
   */
  public function __construct() {
    $this->project_service = \Drupal::service('rawphenotypes.project_service');
    $this->term_service = \Drupal::service('rawphenotypes.term_service');
    $this->default_service = \Drupal::service('rawphenotypes.default_service');
    $this->user_service = \Drupal::service('rawphenotypes.user_service');
  }

  /**
   * Construct spreadsheet.
   */
  public function createFile($project_id = NULL) {
    // Query column headers specific to a project, given a project id.
    if (isset($project_id) AND $project_id > 0) {
      // Array to hold all trait types.
      $trait_type = $this->default_service::getTraitTypes();  
      $cvterm = $this->project_service::getProjectTerms($project_id);

      // Only when project has headers.
      if (count($cvterm) > 0) {
        // Array to hold the column headers passed to the excel writer.
        $col_headers = array();
        // Array to hold standard procedure, which basically is
        // the traits definition and collection method.
        $instructions_data = array();

        // Get the data type per unit. This type will be the cell type in the spreadsheet.
        $data_type = $this->term_service->getUnitDataType('type');

        // Prepend the array with plant property column headers.
        $col_headers = array(
          'Plot'     => 'integer',
          'Entry'    => 'integer',
          'Name'     => 'string',
          'Rep'      => 'integer',
          'Location' => 'string'
        );

        // Start at F column taking into account plant properties.
        // A for Plot, B for Entry and so on (A-E is 5 cols).
        $l = 'F';
        $cell_i = array();

        // Assign the data type for each header based on the unit it contains.
        $h = array('name' => 'Trait', 'definition' => 'Definition', 'method' => 'Collection Method');

        foreach($cvterm as $trait) {
          // Exclude the plant property from the set of headers. They are pre-inserted to the array
          // of column headers passed to the spreadsheet writer.
          if ($trait->type == $trait_type['type4']) continue;

          // Get the unit.
          $u = $this->term_service->getTermUnit($trait->name);
          $unit = isset($data_type[$u]) ? $data_type[$u] : 'string';

          $col_headers[ $trait->name ] = $unit;

          // Highlight the cells when it is essential trait.
          if ($trait->type == $trait_type['type1']) {
            array_push($cell_i, $l . '1');
            // Increment F column.
            $l++;
          }

          // Get header method and definition information.
          $t = $this->term_service->getTermProperties($trait->project_cvterm_id);

          foreach($h as $m_i => $m) {
            $star = ($m_i == 'name') ? '*' : '';

            if (strlen($t[$m_i]) < 80) {
              // Short text, save it.
              array_push($instructions_data, [$star . $m . ':', $t[$m_i]]);
            }
            else {
              // Hard-wrap long lines into shorter line and put each
              // line into a cell/row.
              $wrapped_string = wordwrap($t[$m_i], 100, "\n");
              $chunks = explode("\n", $wrapped_string);

              foreach($chunks as $i => $chunk) {
                $ins_text = ($i == 0) ? [$star . $m . ':', $chunk] : ['', $chunk];
                array_push($instructions_data, $ins_text);
              }
            }
          }

          // Add extra new line.
          array_push($instructions_data, ['' , '']);
        }
        
        // Load spreadsheet writer library.
        $xlsx_writer = libraries_load('spreadsheet_writer');        
        include_once $xlsx_writer['library path'] . '/'. $xlsx_writer['files'][0];

        $writer = new \XLSXWriter();

        // Measurement tab.
        @$writer->writeSheet(array(), 'Measurements', $col_headers,
          array(
            // The entire header row apply these styles.
            array(
              'font' =>
                array(
                  'name'        => 'Arial',
                  'size'        => '11',
                  'color'       => '000000',
                  'bold'        => false,
                  'italic'      => false,
                  'underline'   => false
              ),
              'wrapText'        => true,
              'verticalAlign'   => 'top',
              'horizontalAlign' => 'center',
              'fill'            => array('color' => 'F7F7F7'),
              'border'          => array('style' => 'thin', 'color' => 'A0A0A0'),
              'rows'            => array('0')
            ),
            // Once the styles above have been applied, style the plant property headers.
            array(
              'font' =>
                array(
                  'name'        => 'Arial',
                  'size'        => '11',
                  'color'       => '000000',
                  'bold'        => true,
                  'italic'      => false,
                  'underline'   => false
              ),
              'verticalAlign'   => 'bottom',
              'horizontalAlign' => 'center',
              'fill'            => array('color' => 'EAEAEA'),
              'border'          => array('style' => 'thin', 'color' => 'A0A0A0'),
              'cells'           => array('A1', 'B1', 'C1', 'D1', 'E1')
            ),
            // Make sure to style the essential trait/header.
            array(
              'font' =>
                array(
                  'name'        => 'Arial',
                  'size'        => '11',
                  'color'       => '008000',
                  'bold'        => true,
                  'italic'      => false,
                  'underline'   => false
              ),
              'wrapText'        => true,
              'verticalAlign'   => 'top',
              'horizontalAlign' => 'center',
              'fill'            => array('color' => 'F5FFDF'),
              'border'          => array('style' => 'thin', 'color' => 'A0A0A0'),
              'cells'           => $cell_i
            )
          )
        );

        // Standard procedure tab.
        // Load trait definition and data collection method to this sheet.
        $instructions_header = array();
        @$writer->writeSheet($instructions_data, 'Instructions', $instructions_header,
          array(
            array(
              'font' =>
                array(
                  'name'        => 'Arial',
                  'size'        => '11',
                  'color'       => '000000',
                  'bold'        => true,
                  'italic'      => false,
                  'underline'   => false
              ),
              'wrapText'        => true,
              'columns'         => '0',
            ),
            array(
              'font' =>
                array(
                  'size'        => '12',
                ),
                'wrapText'      => false,
                'columns'       => '1',
            ),
          )
        );

        // Calculator tab.
        $calc_header = array('CALCULATE DAYS TO' => 'string');
        $calc_data =
          array(
            array('Planting Date', '2015-10-06'),
            array('Current Date', date('Y-m-d')),
            array('Current "Days till"', '=B3 - B2'),
            array('',''),
            array('Instructions', ''),
            array('', ''),
            array('Fill out the planting date indicated in the measurements tab, as well as, the current date.', ''),
            array('The "Days till" date will then be calculated for you.', '')
          );

        @$writer->writeSheet($calc_data, 'Calculate Days to', $calc_header,
          array(
            array(
              'font' =>
                array(
                  'name'      => 'Arial',
                  'size'      => '20',
                  'color'     => '000000',
                  'bold'      => true,
                  'italic'    => false,
                  'underline' => false
                ),
                'wrapText'    => false,
                'rows'        => array('0'),
            ),
            array(
              'font' =>
                array(
                  'name'      => 'Arial',
                  'size'      => '11',
                  'color'     => 'FFFFFF',
                  'bold'      => true,
                  'italic'    => false,
                  'underline' => false
                ),
                'wrapText'    => true,
                'fill'        => array('color' => '305673'),
                'border'      => array('style' => 'thin', 'color' => 'A0A0A0'),
                'rows'        => array('1'),
            ),
            array(
              'font' =>
                array(
                  'name'      => 'Arial',
                  'size'      => '11',
                  'color'     => '000000',
                  'bold'      => true,
                  'italic'    => false,
                  'underline' => false
                ),
                'wrapText'    => true,
                'fill'        => array('color' => 'F7F7F7'),
                'border'      => array('style' => 'thin', 'color' => 'A0A0A0'),
                'rows'        => array('2'),
            ),
            array(
              'font' =>
                array(
                  'name'      => 'Arial',
                  'size'      => '11',
                  'color'     => '000000',
                  'bold'      => true,
                  'italic'    => false,
                  'underline' => false
                ),
                'wrapText'    => true,
                'fill'        => array('color' => '79a183'),
                'border'      => array('style' => 'thin', 'color' => 'A0A0A0'),
                'rows'        => array('3'),
            )
          )
        );
        
        // Append user name into the filename.
        // Current user logged in.
        $user_id = \Drupal::currentUser()->id();
        $user = $this->user_service::getUser((int)$user_id, ['name']);

        $filename = 'datacollection_' . $project_id . '_' . str_replace(' ', '_', $user['name']) .'_'. date('YMd') .'_'. time() . '.xlsx';
        $file = file_save_data($writer->writeToString(), 'public://' . $filename);
        
        // Launch save file window and ask user to save file.
        $http_headers = array(
          'Content-Type' => 'application/vnd.openxmlformats-officedocument.spreadsheetml.sheet',
          'Content-Disposition' => 'attachment; filename="' . $filename . '"',
        );

        if (strpos($_SERVER['HTTP_USER_AGENT'], 'MSIE')) {
          $http_headers['Cache-Control'] = 'must-revalidate, post-check=0, pre-check=0';
          $http_headers['Pragma'] = 'public';
        }
        else {
          $http_headers['Pragma'] = 'no-cache';
        }
        
        return [
          '#markup' => '<a href="' . $file->createFileUrl() . '">Download Data Collection Spreadsheet File</a>'
        ];
      }
    }
  }
}