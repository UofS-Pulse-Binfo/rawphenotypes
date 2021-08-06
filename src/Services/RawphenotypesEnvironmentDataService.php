<?php
/**
 * @file
 * Contains class definition of RawphenotypesEnvironment Data Service
 */

namespace Drupal\Rawphenotypes\Services;

use Drupal\file\Entity\File;
use Drupal\Core\Entity\EntityStorageException;


class RawphenotypesEnvironmentDataService {
  /**
   * 
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
   * 
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
   * 
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