<?php
require_once($_SERVER['DOCUMENT_ROOT'] . "/prerequisites.php");

if($selected_controller_id < 1)
{
  header("Location: /");
  exit;
}

$tplEngine->assign("page","controller_settings");
$tplEngine->display("index.tpl");

?>
