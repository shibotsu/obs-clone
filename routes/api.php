<?php

use App\Http\Controllers\ProfileController;
use App\Http\Controllers\RegisteredUserController;
use App\Http\Controllers\SessionController;
use Illuminate\Http\Request;
use Illuminate\Support\Facades\Route;

Route::get('/user', function (Request $request) {
    return $request->user();
})->middleware('auth:sanctum');

Route::post('/register', [RegisteredUserController::class, 'store']);
Route::post('/login', [SessionController::class, 'store']);
Route::post('/logout', [SessionController::class, 'destroy']);
Route::middleware('auth:api')->get('/profile', [ProfileController::class, 'show']);
Route::middleware('auth:api')->post('/picture', [ProfileController::class, 'changePicture']);
Route::get('/profile/{id}',  [ProfileController::class, 'channel']);
Route::get('/most_followed',  [ProfileController::class, 'mostFollowed']);
Route::middleware('auth:api')->group(function () {
    Route::post('/follow/{id}', [ProfileController::class, 'follow']);
    Route::delete('/unfollow/{id}', [ProfileController::class, 'unfollow']);
    Route::get('/{id}/followers', [ProfileController::class, 'followers']);
    Route::get('/following', [ProfileController::class, 'following']);
});

