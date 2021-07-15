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
   */
  public static function addUserToProject($user_id, $project) {
    \Drupal::service('database')
      ->insert('pheno_project_user')
      ->fields([
        'uid' => $user_id,
        'project_id' => $project_id
      ])
      ->execute();
  }

}