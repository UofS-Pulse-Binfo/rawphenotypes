<?php
/**
 * @file
 * Contains class definition of RawphenotypesNotificationService.
 */

namespace Drupal\Rawphenotypes\Services;

class RawphenotypesNotificationService {
  /**
   * Post a TRIPAL CRITICAL report error message.
   * 
   * @param $message
   *   String, message body of the error.
   */
  public static function postTripalReportError($message) {
    $module = 'Raw Phenotypes';
    $message = trim($message);
    
    tripal_report_error($module, TRIPAL_CRITICAL, $message, [], ['print' => TRUE]);
  }
}