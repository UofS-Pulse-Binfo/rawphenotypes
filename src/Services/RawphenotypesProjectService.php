<?php
/**
 * @file
 * Contains class definition of RawphenotypesProjectService.
 */

namespace Drupal\Rawphenotypes\Services;

class RawphenotypesProjectService {
  /**
   * Get project.
   * 
   * @param $project_name
   *   String, project name or title to search.
   * 
   * @return object
   *   Project row object.
   */
  public static function getProjectByName($project_name) {
    // project_id, name and description.
    $project_name = trim($project_name);

    $sql = "SELECT * FROM chado.project WHERE name = :project_name";
    $args = [':project_name' => $project_name];

    $project = \Drupal::database()
      ->query($sql, $args);
    
    $project->allowRowCount = TRUE;
    
    return ($project->rowCount()) ? $project->fetchObject() : null;
  }

  /**
   * Add user to a project.
   * 
   * @param $user_id
   *   Integer, Drupal user user id.
   * @param $project_id
   *   Integer, project id number.
   */
  public static function addUserToProject($user_id, $project_id) {
    $table = 'pheno_project_user';

    $sql = sprintf("SELECT project_user_id FROM %s WHERE uid = :user AND project_id = :project", $table);
    $args = [':user' => $user_id, ':project' => $project_id];

    $query = \Drupal::database()
      ->query($sql, $args);
    
    $query->allowRowCount = TRUE;

    if ($query->rowCount() <= 0) { 
      // Only when user and project does not exist.
      \Drupal::service('database')
        ->insert($table)
        ->fields([
          'uid' => $user_id,
          'project_id' => $project_id
        ])
        ->execute();    
    }
  }




}