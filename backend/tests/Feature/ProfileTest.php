<?php

namespace Tests\Feature;

use App\Models\User;
use Illuminate\Foundation\Testing\RefreshDatabase;
use Illuminate\Foundation\Testing\WithFaker;
use Illuminate\Support\Facades\Hash;
use Tests\TestCase;
use Tymon\JWTAuth\Facades\JWTAuth;

class ProfileTest extends TestCase
{
    use RefreshDatabase;

    public function test_authenticated_user_can_view_own_profile()
    {
        $user = User::factory()->create();
        $token = JWTAuth::fromUser($user);

        $response = $this->withHeader('Authorization', "Bearer $token")
            ->getJson('/api/profile');

        $response->assertStatus(200)
            ->assertJsonFragment(['id' => $user->id]);
    }

    public function test_unauthenticated_user_cannot_view_own_profile()
    {
        $response = $this->getJson('/api/profile');

        $response->assertStatus(401);
    }

    public function test_authenticated_user_can_change_profile_picture()
    {
        $user = User::factory()->create();
        $token = JWTAuth::fromUser($user);

        // Create a fake image
        $file = \Illuminate\Http\UploadedFile::fake()->image('avatar.jpg');

        $response = $this->withHeader('Authorization', "Bearer $token")
            ->postJson('/api/picture', [
                'profile_picture' => $file
            ]);

        $response->assertStatus(200)
            ->assertJsonStructure(['message', 'url']);
        $this->assertDatabaseHas('users', [
            'id' => $user->id,
        ]);
    }

    public function test_unauthenticated_user_cannot_change_profile_picture()
    {
        $response = $this->postJson('/api/picture', [
            'profile_picture' => 'new_picture_url.jpg'
        ]);
        $response->assertStatus(401);
    }

    public function test_can_view_another_users_profile()
    {
        $user = User::factory()->create([
            'username' => 'testuser',
            'number_of_followers' => 5,
            'profile_picture' => 'profile_pictures/avatar.jpg'
        ]);

        $response = $this->getJson("/api/profile/{$user->id}");

        $response->assertStatus(200)
            ->assertJson([
                'channel' => [
                    'username' => 'testuser',
                    'number_of_followers' => 5,
                    'profile_picture' => asset('storage/profile_pictures/avatar.jpg'),
                ]
            ]);
    }

    public function test_view_nonexistent_user_profile_returns_404()
    {
        $response = $this->getJson("/api/profile/99999");
        $response->assertStatus(404);
    }

    public function test_can_search_users()
    {
        $user1 = User::factory()->create(['username' => 'alice']);
        $user2 = User::factory()->create(['username' => 'bob']);

        $response = $this->postJson('/api/search', [
            'query' => 'ali'
        ]);

        $response->assertStatus(200)
            ->assertJsonFragment(['username' => 'alice'])
            ->assertJsonMissing(['username' => 'bob']);
    }

    public function test_authenticated_user_can_edit_username()
    {
        $user = User::factory()->create(['username' => 'oldname']);
        $token = JWTAuth::fromUser($user);

        $response = $this->withHeader('Authorization', "Bearer $token")
            ->putJson('/api/usernameupdate', [
                'username' => 'oldname',
                'new_username' => 'newname'
            ]);

        $response->assertStatus(200)
            ->assertJsonFragment(['message' => 'Username successfully updated']);
        $this->assertDatabaseHas('users', [
            'id' => $user->id,
            'username' => 'newname'
        ]);
    }

    public function test_edit_username_wrong_current_username()
    {
        $user = User::factory()->create(['username' => 'oldname']);
        $token = JWTAuth::fromUser($user);

        $response = $this->withHeader('Authorization', "Bearer $token")
            ->putJson('/api/usernameupdate', [
                'username' => 'wrongname',
                'new_username' => 'newname'
            ]);

        $response->assertStatus(400)
            ->assertJson(['Wrong username']);
    }

    public function test_authenticated_user_can_edit_email()
    {
        $user = User::factory()->create(['email' => 'old@example.com']);
        $token = JWTAuth::fromUser($user);

        $response = $this->withHeader('Authorization', "Bearer $token")
            ->putJson('/api/emailupdate', [
                'email' => 'old@example.com',
                'new_email' => 'new@example.com'
            ]);

        $response->assertStatus(200)
            ->assertJsonFragment(['message' => 'Email successfully updated']);
        $this->assertDatabaseHas('users', [
            'id' => $user->id,
            'email' => 'new@example.com'
        ]);
    }

    public function test_edit_email_wrong_current_email()
    {
        $user = User::factory()->create(['email' => 'old@example.com']);
        $token = JWTAuth::fromUser($user);

        $response = $this->withHeader('Authorization', "Bearer $token")
            ->putJson('/api/emailupdate', [
                'email' => 'wrong@example.com',
                'new_email' => 'new@example.com'
            ]);

        $response->assertStatus(400)
            ->assertJson(['Wrong email']);
    }

    public function test_authenticated_user_can_edit_password()
    {
        $user = User::factory()->create(['password' => bcrypt('oldpassword')]);
        $token = JWTAuth::fromUser($user);

        $response = $this->withHeader('Authorization', "Bearer $token")
            ->putJson('/api/passwordupdate', [
                'password' => 'oldpassword',
                'new_password' => 'newpassword'
            ]);

        $response->assertStatus(200)
            ->assertJsonFragment(['message' => 'Password successfully updated']);
        $user->refresh();
        $this->assertTrue(Hash::check('newpassword', $user->password));
    }

    public function test_edit_password_wrong_current_password()
    {
        $user = User::factory()->create(['password' => bcrypt('oldpassword')]);
        $token = JWTAuth::fromUser($user);

        $response = $this->withHeader('Authorization', "Bearer $token")
            ->putJson('/api/passwordupdate', [
                'password' => 'wrongpassword',
                'new_password' => 'newpassword'
            ]);

        $response->assertStatus(400)
            ->assertJson(['message' => 'Wrong password']);
    }

    public function test_authenticated_user_can_destroy_account()
    {
        $user = User::factory()->create();
        $token = \Tymon\JWTAuth\Facades\JWTAuth::fromUser($user);

        $response = $this->withHeader('Authorization', "Bearer $token")
            ->deleteJson('/api/userdelete');

        $response->assertStatus(200);
        $this->assertDatabaseMissing('users', [
            'id' => $user->id
        ]);
    }

    public function test_search_returns_no_results_for_non_matching_query()
    {
        User::factory()->create(['username' => 'alice']);
        $response = $this->postJson('/api/search', ['query' => 'nonexistentuser']);
        $response->assertStatus(200)
            ->assertJsonMissing(['username' => 'alice'])
            ->assertJsonCount(0, 'users');
    }

    public function test_search_is_case_insensitive()
    {
        User::factory()->create(['username' => 'Alice']);
        $response = $this->postJson('/api/search', ['query' => 'alice']);
        $response->assertStatus(200)
            ->assertJsonFragment(['username' => 'Alice']);
    }

    public function test_search_handles_special_characters()
    {
        User::factory()->create(['username' => 'user.name']);
        $response = $this->postJson('/api/search', ['query' => 'user.name']);
        $response->assertStatus(200)
            ->assertJsonFragment(['username' => 'user.name']);
    }

    public function test_unauthenticated_user_cannot_delete_account()
    {
        $response = $this->deleteJson('/api/userdelete');
        $response->assertStatus(401);
    }


    public function test_user_cannot_delete_twice()
    {
        $user = User::factory()->create();
        $token = JWTAuth::fromUser($user);

        $this->withHeader('Authorization', "Bearer $token")
            ->deleteJson('/api/userdelete')
            ->assertStatus(200);

        // Try again with the same token (should fail)
        $this->withHeader('Authorization', "Bearer $token")
            ->deleteJson('/api/userdelete')
            ->assertStatus(401); // Or 404, depending on your logic
    }
}
