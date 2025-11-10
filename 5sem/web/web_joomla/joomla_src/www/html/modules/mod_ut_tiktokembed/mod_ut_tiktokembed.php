<?php
/**
 * @package Joomla.Site
 * @subpackage mod_ut_tiktokembed
 * @author Unitemplates https://www.unitemplates.com
 * @copyright Copyright (C) 2025 - 2025 Unitemplates All rights reserved.
 * @license GNU General Public License version 2 or later; see LICENSE.txt
 */

defined('_JEXEC') or die;

use Joomla\CMS\Helper\ModuleHelper;
use Joomla\Module\UtTiktokEmbed\Site\Helper\TiktokEmbedHelper;

// Get module parameters
$embedType = $params->get('embed_type', 'single');
$tiktokUrl = $params->get('tiktok_url', '');
$tiktokList = $params->get('tiktok_list', '');
$moduleclass_sfx = htmlspecialchars($params->get('moduleclass_sfx', ''), ENT_COMPAT, 'UTF-8');

// Get the embed HTML from TikTok
$embedHtml = TiktokEmbedHelper::getEmbed($tiktokUrl, $embedType, $tiktokList);

// Display the layout
require ModuleHelper::getLayoutPath('mod_ut_tiktokembed', $params->get('layout', 'default'));