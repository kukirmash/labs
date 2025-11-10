<?php

/**
 * @package Joomla.Site
 * @subpackage mod_out_articles
 *
 * @copyright Copyright (C) 2005 - 2016 Open Source Matters,
 * Inc. All rights reserved.
 * @license GNU General Public License version 2 or later;
 * see LICENSE.txt
 */

defined('_JEXEC') or die;

use Joomla\CMS\Language\Text;
use Joomla\CMS\HTML\HTMLHelper;
use Joomla\CMS\Factory;

// Приветствуем текущего пользователя
$user = Factory::getUser();

if ($user->guest) {
    echo "<p>Привет, Гость!</p>";
} else {
    echo "<p>Привет, " . htmlspecialchars($user->name) . "!</p>";
}

?>