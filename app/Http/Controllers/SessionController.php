<?php

namespace App\Http\Controllers;

use Illuminate\Http\Request;
use Illuminate\Support\Facades\Auth;
use Illuminate\Validation\ValidationException;
use Tymon\JWTAuth\Facades\JWTAuth;

class SessionController extends Controller
{

    public function store()
    {
        $credentials = request()->validate([
            'login' => ['required', 'string'],
            'password' => ['required', 'string', 'min:8'],
        ]);

        try {
            $login_type = filter_var($credentials['login'], FILTER_VALIDATE_EMAIL) ? 'email' : 'username';

            if (!$token = JWTAuth::attempt([
                $login_type => $credentials['login'],
                'password' => $credentials['password'],
            ])) {
                return response()->json(['error' => 'Invalid credentials'], 401);
            }
            $user = Auth::user();
            return response()->json(['token' => $token, 'user' => $user], 200);
        } catch (\Exception $e) {
            return response()->json(['error' => 'Could not create token'], 500);
        }
    }

    public function destroy() {
        try {
            JWTAuth::invalidate(JWTAuth::getToken());

            return response()->json(['message' => 'Successfully logged out'], 200);
        } catch (\Exception $e) {
            return response()->json(['error' => 'Failed to log out'], 500);
        }
    }
}
