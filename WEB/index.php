<?php
require_once($_SERVER['DOCUMENT_ROOT'] . "/prerequisites.php");

$tplEngine->assign("page","main");
$tplEngine->display("index.tpl");

?>
