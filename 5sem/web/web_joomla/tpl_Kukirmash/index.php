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
$radius		= $this->params->get('radius');
$shadow		= $this->params->get('shadow');

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

	<!-- Bootstrap JS -->
	<script src="<?php echo $template_path; ?>/js/bootstrap.min.js"></script>


	<!-- Template styles -->
	<link rel="stylesheet" href="<?php echo $template_path; ?>/css/template.css">
	<link rel="stylesheet" href="<?php echo $template_path; ?>/css/radius-<?php echo $radius; ?>.css">
	<link rel="stylesheet" href="<?php echo $template_path; ?>/css/<?php echo $color; ?>.css">
	<?php if ($shadow == '0') : ?>
		<link rel="stylesheet" href="<?php echo $template_path; ?>/css/shadow.css">
	<?php endif; ?>

	<link rel="preconnect" href="https://fonts.googleapis.com">
	<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
	<link href="https://fonts.googleapis.com/css2?family=M+PLUS+1p&display=swap" rel="stylesheet">
	<link href="https://fonts.googleapis.com/css2?family=Science+Gothic:wght@100..900&display=swap" rel="stylesheet">
</head>

<body id="body">
	<div class="container" id="body-container">

		<!-- =================== HEADER =================== -->
		<header>
			<div class="row" id="header">

				<!-- Логотип (3 колонки) -->
				<div class="col-3" id="header-logo">
					<?php if ($logo) : ?>
						<img src="<?php echo htmlentities($logo, ENT_QUOTES, 'UTF-8'); ?>" class="img-fluid">
					<?php else : ?>
						<h1 id="logo-text"><?php echo $sitename; ?></h1>
					<?php endif; ?>
				</div>
					
				<?php if ($color == "christmas") : ?>
					<div id="carouselExample" class="carousel slide">
						<div class="carousel-inner">
							<div class="carousel-item active">
								<img height="600px" src="https://i.pinimg.com/1200x/2b/c5/6e/2bc56e1e6564ea2018ed7cdac3572fb5.jpg" class="d-block w-100" alt="...">
							</div>
							<div class="carousel-item">
								<img height="600px" src="https://i.pinimg.com/736x/39/0e/02/390e02b9e53d6aa6a7c312068eff4ca8.jpg" class="d-block w-100" alt="...">
							</div>
							<div class="carousel-item">
								<img height="600px" src="https://i.pinimg.com/736x/c9/ea/c8/c9eac8f3b148601dbbef1f97522bd7e4.jpg" class="d-block w-100" alt="...">
							</div>
							<div class="carousel-item">
								<img height="600px " src="https://i.pinimg.com/originals/52/ce/57/52ce57e7e3cbb5a31cc7792180d734d9.gif" class="d-block w-100" alt="...">
							</div>
							<div class="carousel-item">
								<img height="600px" src="https://i.pinimg.com/736x/77/60/8f/77608ffee78946396ccd1955fabe6a5a.jpg" class="d-block w-100" alt="...">
							</div>
							<div class="carousel-item">
								<img height="600px" src="https://i.pinimg.com/736x/ab/0e/75/ab0e75d0a76640ead0d647d6eebd6950.jpg" class="d-block w-100" alt="...">
							</div>
						</div>
						<button class="carousel-control-prev" type="button" data-bs-target="#carouselExample" data-bs-slide="prev">
							<span class="carousel-control-prev-icon" aria-hidden="true"></span>
							<span class="visually-hidden">Previous</span>
						</button>
						<button class="carousel-control-next" type="button" data-bs-target="#carouselExample" data-bs-slide="next">
							<span class="carousel-control-next-icon" aria-hidden="true"></span>
							<span class="visually-hidden">Next</span>
						</button>
					</div>
				<?php endif; ?>

				<!-- Позиция header -->
				<?php if ($this->countModules('kukirmash-header')) : ?>
					<jdoc:include type="modules" name="kukirmash-header" />
				<?php endif; ?>

			</div>
		</header>

		<!-- =================== МЕНЮ =================== -->
		<div class="row">
			<section class="col-12 g-4" id="topmenu">
				<?php if ($this->countModules('kukirmash-topmenu')) : ?>
					<jdoc:include type="modules" name="kukirmash-topmenu" />
				<?php endif; ?>
			</section>
		</div>

		<!-- =================== ОСНОВНОЕ СОДЕРЖИМОЕ =================== -->
		<main>
			<div class="row">
				<!-- Левая колонка (3/12) -->
				<aside class="col-3 g-4" id="left-sidebar">
					<?php if ($this->countModules('kukirmash-left')) : ?>
						<jdoc:include type="modules" name="kukirmash-left" />
					<?php endif; ?>
				</aside>

				<!-- Основное содержимое (6/12) -->
				<section class="col-6 g-4" id="main-content">
					<jdoc:include type="component" />
				</section>

				<!-- Правая колонка (3/12) -->
				<aside class="col-3 g-4" id="right-sidebar">
					<?php if ($this->countModules('kukirmash-right')) : ?>
						<jdoc:include type="modules" name="kukirmash-right" />
					<?php endif; ?>
				</aside>

			</div>
		</main>

		<!-- =============== БЛОК ДЛЯ МОДУЛЯ =============== -->
		<div class="row">
			<section class="col-12 g-4" id="module">
				<?php if ($this->countModules('kukirmash-module')) : ?>
					<jdoc:include type="modules" name="kukirmash-module" />
				<?php endif; ?>
			</section>
		</div>

		<!-- =============== ПОДВАЛ =============== -->
		<div class="row">
			<section class="col-12 g-4" id="footer">
				<?php if ($color == "bellflowers") : ?>
					<div class="accordion" id="accordionExample">
						<div class="accordion-item">
							<h2 class="accordion-header">
								<button class="accordion-button" type="button" data-bs-toggle="collapse" data-bs-target="#collapseOne" aria-expanded="true" aria-controls="collapseOne">
									Элемент аккордеона #1
								</button>
							</h2>
							<div id="collapseOne" class="accordion-collapse collapse show" data-bs-parent="#accordionExample">
								<div class="accordion-body">
									<strong>Это тело первого элемента аккордеона.</strong> Оно отображается по умолчанию, пока плагин collapse не добавит соответствующие классы, которые мы используем для стилизации каждого элемента. Эти классы контролируют общий внешний вид, а также показ и скрытие через CSS переходы. Вы можете изменить любое из этого с помощью пользовательского CSS или переопределения наших переменных по умолчанию. Также стоит отметить, что практически любой HTML может быть внутри <code>.accordion-body</code>, хотя переход ограничивает переполнение.
								</div>
							</div>
						</div>
						<div class="accordion-item">
							<h2 class="accordion-header">
								<button class="accordion-button collapsed" type="button" data-bs-toggle="collapse" data-bs-target="#collapseTwo" aria-expanded="false" aria-controls="collapseTwo">
									Элемент аккордеона #2
								</button>
							</h2>
							<div id="collapseTwo" class="accordion-collapse collapse" data-bs-parent="#accordionExample">
								<div class="accordion-body">
									<strong>Это тело второго элемента аккордеона.</strong> Оно скрыто по умолчанию, пока плагин collapse не добавит соответствующие классы, которые мы используем для стилизации каждого элемента. Эти классы контролируют общий внешний вид, а также показ и скрытие через CSS переходы. Вы можете изменить любое из этого с помощью пользовательского CSS или переопределения наших переменных по умолчанию. Также стоит отметить, что практически любой HTML может быть внутри <code>.accordion-body</code>, хотя переход ограничивает переполнение.
								</div>
							</div>
						</div>
						<div class="accordion-item">
							<h2 class="accordion-header">
								<button class="accordion-button collapsed" type="button" data-bs-toggle="collapse" data-bs-target="#collapseThree" aria-expanded="false" aria-controls="collapseThree">
									Элемент аккордеона #3
								</button>
							</h2>
							<div id="collapseThree" class="accordion-collapse collapse" data-bs-parent="#accordionExample">
								<div class="accordion-body">
									<strong>Это тело третьего элемента аккордеона.</strong> Оно скрыто по умолчанию, пока плагин collapse не добавит соответствующие классы, которые мы используем для стилизации каждого элемента. Эти классы контролируют общий внешний вид, а также показ и скрытие через CSS переходы. Вы можете изменить любое из этого с помощью пользовательского CSS или переопределения наших переменных по умолчанию. Также стоит отметить, что практически любой HTML может быть внутри <code>.accordion-body</code>, хотя переход ограничивает переполнение.
								</div>
							</div>
						</div>
					</div>
				<?php endif; ?>

				<?php if ($this->countModules('kukirmash-footer')) : ?>
					<jdoc:include type="modules" name="kukirmash-footer" />
				<?php endif; ?>
			</section>
		</div>

		<!-- =============== =============== =============== -->
	</div>
</body>

</html>