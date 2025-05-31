<?php

namespace Tests\Feature;

use App\Models\User;
use Illuminate\Foundation\Testing\RefreshDatabase;
use Illuminate\Foundation\Testing\WithFaker;
use Tests\TestCase;
use Tymon\JWTAuth\Facades\JWTAuth;

class AuthTest extends TestCase
{
    use RefreshDatabase;

    public function test_user_can_register()
    {
        $response = $this->postJson('/api/register', [
            'username' => 'john123',
            'email' => 'john@example.com',
            'password' => 'secret123',
            'birthday' => '1990-01-01',
            'number_of_followers' => '1',
            'profile_picture' => 'none'
        ]);

        $response->assertStatus(201);
        $this->assertDatabaseHas('users', ['email' => 'john@example.com']);
        $response->assertJsonStructure(['token']);
    }

    public function test_user_can_login_with_email()
    {
        $user = User::factory()->create(['password' => bcrypt('password')]);

        $response = $this->postJson('/api/login', [
            'login' => $user->email,
            'password' => 'password',
        ]);

        $response->assertStatus(200);
        $response->assertJsonStructure(['token']);
    }
    public function test_user_can_login_with_username()
    {
        $user = User::factory()->create(['password' => bcrypt('password')]);

        $response = $this->postJson('/api/login', [
            'login' => $user->username,
            'password' => 'password',
        ]);

        $response->assertStatus(200);
        $response->assertJsonStructure(['token']);
    }
    public function test_user_can_logout_successfully()
    {

        $user = User::factory()->create();
        // Get JWT token for user
        $token = JWTAuth::fromUser($user);
        // Make the logout request with Authorization header
        $response = $this->withHeaders([
            'Authorization' => 'Bearer ' . $token,
        ])->postJson('/api/logout');
        $response->assertStatus(200);
        $response->assertJson([
            'message' => 'Successfully logged out'
        ]);
    }
    public function test_register_fails_with_missing_fields()
    {
        $response = $this->postJson('/api/register', []);
        $response->assertStatus(422);
        $response->assertJsonValidationErrors(['username', 'email', 'password', 'birthday']);
    }

    public function test_register_fails_with_duplicate_email()
    {
        User::factory()->create(['email' => 'john@example.com']);
        $response = $this->postJson('/api/register', [
            'username' => 'john123',
            'email' => 'john@example.com',
            'password' => 'secret123',
            'birthday' => '1990-01-01',
            'number_of_followers' => '1',
            'profile_picture' => 'none'
        ]);
        $response->assertStatus(422);
        $response->assertJsonValidationErrors(['email']);
    }

    public function test_register_fails_with_invalid_email()
    {
        $response = $this->postJson('/api/register', [
            'username' => 'john123',
            'email' => 'not-an-email',
            'password' => 'secret123',
            'birthday' => '1990-01-01',
            'number_of_followers' => '1',
            'profile_picture' => 'none'
        ]);
        $response->assertStatus(422);
        $response->assertJsonValidationErrors(['email']);
    }

    public function test_login_fails_with_wrong_password()
    {
        $user = User::factory()->create(['password' => bcrypt('password')]);
        $response = $this->postJson('/api/login', [
            'login' => $user->email,
            'password' => 'wrongpassword',
        ]);
        $response->assertStatus(401);
    }

    public function test_login_fails_with_nonexistent_user()
    {
        $response = $this->postJson('/api/login', [
            'login' => 'nonexistent@example.com',
            'password' => 'password',
        ]);
        $response->assertStatus(401);
    }

    public function test_login_fails_with_empty_fields()
    {
        $response = $this->postJson('/api/login', [
            'login' => '',
            'password' => '',
        ]);
        $response->assertStatus(422);
        $response->assertJsonValidationErrors(['login', 'password']);
    }

    public function test_logout_fails_with_invalid_token()
    {
        $response = $this->withHeaders([
            'Authorization' => 'Bearer invalidtoken',
        ])->postJson('/api/logout');
        $response->assertStatus(401);
    }

    public function test_logout_fails_with_missing_token()
    {
        $response = $this->postJson('/api/logout');
        $response->assertStatus(401);
    }
}
