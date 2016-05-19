<?php
require_once($_SERVER['DOCUMENT_ROOT'] . "/prerequisites.php");

$controller_id = intval(@$_GET['id']);
if($controller_id < 1)
{
  header("Location: /");
  exit;
}

$selected_controller = null;
foreach($controllers as $k => $v)
{
  if($v['controller_id'] == $controller_id)
  {
    $selected_controller = $v;
    break;
  }
}

$tplEngine->assign("selected_controller", $selected_controller);
$tplEngine->assign("page","controller");
$tplEngine->display("index.tpl");

?>
