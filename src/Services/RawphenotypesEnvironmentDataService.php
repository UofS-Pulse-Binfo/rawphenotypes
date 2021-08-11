<?php
/**
 * @file
 * Contains class definition of RawphenotypesEnvironment Data Service
 */

namespace Drupal\Rawphenotypes\Services;

use Drupal\file\Entity\File;

class RawphenotypesEnvironmentDataService {
  /**
   * Create a record of an environment data file.
   * Keys:
   * - project_id: project to which a file belongs to.
   * - fid: file id number (Drupal file entity).
   * - location: location consistent with the data in the file.
   * - year: year indicated in the data file.
   * - sequence_no: multiple files can be uploaded give a project-location-year
   * combinateion. This field will number in sequence order each entry.
   * 
   * @param $details
   *   Array, with the key corresponding to field in pheno_environement_data table.
   */
  public static function saveEnvData($details) {
    $table = 'pheno_environment_data';
    // Create a row in the table.
    \Drupal::database()
      ->insert($table)
      ->fields([
        'project_id' => $details['project_id'],
        'fid'       => $details['fid'],
        'location' => $details['location'],
        'year'    => $details['year'],
        'sequence_no' => $details['sequence_no']
      ])
    ->execute();
  }

  /**
   * Calculate the next sequence order of a file given a
   * project-location-year combination.
   * 
   * @param $project_id
   *   Integer, project id number.
   * @param $location
   *   String, location information the file is specific to.
   * @param $year
   *   Year value, in relation to location and project, the year data was collected.
   * 
   * @return integer
   *   Next sequence number or 1 to indicate the first sequence when none was uploaded.
   */
  public static function getSequenceNumber($project_id, $location, $year) {
    // Rename file to include location, year, sequence no.
    // Fetch the largest seq no in a project, location and year.
    $sql = "
      SELECT sequence_no FROM pheno_environment_data
      WHERE location = :location AND year = :year AND project_id = :project_id
      ORDER BY sequence_no DESC LIMIT 1
    ";
    $args = [
      ':project_id' => $project_id,
      ':location' => $location,
      ':year'   => $year,
    ];

    $result = \Drupal::database()
      ->query($sql, $args);
    
    $result->allowRowCount = TRUE;
    
    return ($result->rowCount()) ? $result->fetchField() + 1 : 1;
  }

  /**
   * Remove environment data file entry from pheno_environment_data table.
   * Physical file is still available in the Drupal public:// file system.
   * 
   * @param $envdata
   *   Array, containing the record id, location, year and project_id.
   */
  public static function deleteEnvData($envdata) {
    $table = 'pheno_environment_data';

    \Drupal::database()
      ->delete($table)
      ->condition('environment_data_id', $envdata['envdata_id'])
      ->execute();

    $file_entity = File::load($envdata['fid']);
    file_delete($file_entity->id());

    $sql = "
      SELECT environment_data_id, sequence_no 
      FROM pheno_environment_data
      WHERE location = :location AND year = :year AND project_id = :project_id
      ORDER BY sequence_no ASC
    ";
    $args = [
      ':project_id' => $envdata['project_id'],
      ':location' => $envdata['location'],
      ':year' => $envdata['year'],
    ];

    $result = \Drupal::database()
      ->query($sql, $args);
    
    $result->allowRowCount = TRUE;
    
    if ($result->rowCount()) {
      $seq = 1;
      foreach($result as $id => $seq_no) {
        \Drupal::database()
          ->update($table)
          ->fields([
            'sequence_no' => $seq + 1
          ])
          ->condition('environment_data_id', $id, '=')
          ->execute();

        $sql++;
      }
    }
  }
}