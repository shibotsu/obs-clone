<?php

namespace App\Http\Controllers;

use App\Models\User;
use Illuminate\Http\Request;
use Illuminate\Support\Facades\Auth;
use Tymon\JWTAuth\Facades\JWTAuth;


class RegisteredUserController extends Controller
{

    public function store() {
        $validatedAttributes = request()->validate([
            'username' => ['required', 'string', 'max:255', 'unique:users,username'],
            'email' => ['required', 'string', 'email', 'max:255', 'unique:users,email'],
            'password' => ['required', 'string', 'min:8'],
            'birthday' => ['required', 'date', 'before:today'],
        ]);

        $user = User::create($validatedAttributes);
        $token = JWTAuth::fromUser($user);
        return response()->json(['token' => $token], 201);
    }

}
