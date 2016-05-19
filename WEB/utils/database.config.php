<?php
if (!defined('IN_SERVER')) { exit; }


class GreenhouseDB extends SQLite3
{
    function __construct()
    {
       $this->open(DBNAME);
    }
    
    
    function beginTransaction()
    {
      $this->exec("BEGIN IMMEDIATE;");
    }
    
    function commitTransaction()
    {
      $this->exec("COMMIT;");
    }
}



$dbengine = new GreenhouseDB();
$dbengine->busyTimeout(5000);
$dbengine->exec('PRAGMA journal_mode=WAL;');

?>