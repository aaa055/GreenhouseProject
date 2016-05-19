<?php
if (!defined('IN_SERVER')) { exit(); }


require_once(INCLUDE_PATH . 'smarty/Smarty.class.php');


class GHSmarty extends Smarty
{

	function GHSmarty()
	{

		$this->Smarty();

		$this->template_dir = INCLUDE_PATH . 'templates/';
		$this->compile_dir = INCLUDE_PATH .  'templates_c/';
		$this->config_dir = INCLUDE_PATH . 'configs/';
		$this->cache_dir = INCLUDE_PATH . 'cache/';
		
	}

};

// создаём экземпляр нашего шаблонного движка
$tplEngine = new GHSmarty();

?>