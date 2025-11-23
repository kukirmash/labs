<?php

/**
 * @package     Joomla.Site
 * @subpackage  Templates.Strict_color
 * 
 * @copyright   (C) 2017 Open Source Matters, Inc. <https://www.joomla.org>
 * @license     GNU General Public License version 2 or later; see LICENSE.txt
 */

// No direct access.
defined('_JEXEC') or die;

use Joomla\CMS\Factory;

// Get params
$logo		= $this->params->get('logo');
$color      = $this->params->get('templatecolor');

// Template path & bootstrap on Joomla's site path
$template_path  = $this->baseurl . '/templates/' . $this->template;
$bootstrap_path = $this->baseurl . '/media/vendor/bootstrap/css';
$bootstrap_js_path = $this->baseurl . '/media/vendor/bootstrap/js';

// Site name for footer
$app = Factory::getApplication();
$sitename = htmlentities($app->get('sitename'), ENT_QUOTES, 'UTF-8');
?>

<!DOCTYPE html>
<html lang="<?php echo $this->language; ?>"
	dir="<?php echo $this->direction; ?>">

<head>
	<jdoc:include type="head" />

	<!-- Bootstrap CSS -->
	<link rel="stylesheet" href="<?php echo $template_path; ?>/css/bootstrap.min.css">

	<!-- Template styles -->
	<link rel="stylesheet" href="<?php echo $template_path; ?>/css/template.css">
	<link rel="stylesheet" href="<?php echo $template_path; ?>/css/<?php echo $color; ?>.css">

	<!-- Bootstrap JS -->
	<script src="<?php echo $template_path; ?>/js/bootstrap.bundle.min.js"></script>

	<!-- jQuery (если нужно) -->
	<script src="https://code.jquery.com/jquery-3.6.0.min.js"></script>
</head>

<body id="body">
	<div class="container" id="body-container">

		<!-- =================== HEADER =================== -->
		<header>
			<div class="row" id="header" >

				<!-- Логотип (3 колонки) -->
				<div class="col-3" id="header-logo">
					<?php if ($logo) : ?>
						<img src="<?php echo htmlentities($logo, ENT_QUOTES, 'UTF-8'); ?>" class="img-fluid">
					<?php else : ?>
						<h1 id="logo-text"><?php echo $sitename; ?></h1>
					<?php endif; ?>
				</div>
					
				<!-- Позиция header -->
				<?php if ($this->countModules('kukirmash-header')) : ?>
					<jdoc:include type="modules" name="kukirmash-header" />
				<?php endif; ?>

			</div>
		</header>
		
		<!-- =================== МЕНЮ =================== -->
		<div class="row">
			<section class="col-12" id="topmenu">
				<?php if ($this->countModules('kukirmash-topmenu')) : ?>
					<jdoc:include type="modules" name="kukirmash-topmenu" />
				<?php endif; ?>
			</section>
		</div>

		<!-- =================== ОСНОВНОЕ СОДЕРЖИМОЕ =================== -->
		<main>
			<div class="row">
				<!-- Левая колонка (3/12) -->
				<aside class="col-3" id="left-sidebar">
					<?php if ($this->countModules('kukirmash-left')) : ?>
						<jdoc:include type="modules" name="kukirmash-left" />
					<?php endif; ?>
				</aside>

				<!-- Основное содержимое (6/12) -->
				<section class="col-6" id="main-content">
					<jdoc:include type="component" />
				</section>

				<!-- Правая колонка (3/12) -->
				<aside class="col-3" id="right-sidebar">
					<?php if ($this->countModules('kukirmash-right')) : ?>
						<jdoc:include type="modules" name="kukirmash-right" />
					<?php endif; ?>
				</aside>

			</div>
		</main>

		<!-- =============== БЛОК ДЛЯ МОДУЛЯ =============== -->
		<div class="row">
			<section class="col-12" id="module">
				<?php if ($this->countModules('kukirmash-module')) : ?>
					<jdoc:include type="modules" name="kukirmash-module" />
				<?php endif; ?>
			</section>
		</div>
		
		<!-- =============== ПОДВАЛ =============== -->
		<div class="row">
			<section class="col-12" id="footer">
				<p>Подвал</p>
				<?php if ($this->countModules('kukirmash-footer')) : ?>
					<jdoc:include type="modules" name="kukirmash-footer" />
				<?php endif; ?>
			</section>
		</div>

		<!-- =============== =============== =============== -->
	</div>
</body>

</html>